#!/usr/bin/env python3
"""
Minimal reference multiplayer TCP server for local development.

- Line-delimited JSON protocol.
- Adds 'protocol' and 'seq' fields to server messages.
- Responds to 'ping' with 'pong'.
- Broadcasts chat and simple presence messages.
- Maintains an incrementing sequence number for zone syncs.
"""

import asyncio
import json
from asyncio import StreamReader, StreamWriter

PROTOCOL_VERSION = 1

class Server:
    def __init__(self, host='127.0.0.1', port=12345):
        self.host = host
        self.port = port
        self.clients = {}  # writer -> username
        self.seq = 0
        self.lock = asyncio.Lock()

    async def start(self):
        server = await asyncio.start_server(self.handle_client, self.host, self.port)
        addr = server.sockets[0].getsockname()
        print(f"Reference server listening on {addr}")
        async with server:
            await server.serve_forever()

    async def broadcast(self, message_obj):
        message_obj['protocol'] = PROTOCOL_VERSION
        # server seq for ordering important messages (zone updates)
        if message_obj.get('type') in ('zone_sync',):
            async with self.lock:
                self.seq += 1
                message_obj['seq'] = self.seq
        data = (json.dumps(message_obj, separators=(',', ':')) + "\n").encode('utf-8')
        for w in list(self.clients.keys()):
            try:
                w.write(data)
                await w.drain()
            except Exception:
                # client likely disconnected; will be cleaned elsewhere
                pass

    async def handle_client(self, reader: StreamReader, writer: StreamWriter):
        peer = writer.get_extra_info('peername')
        print(f"Client connected: {peer}")
        username = None
        self.clients[writer] = None
        try:
            while not reader.at_eof():
                line = await reader.readline()
                if not line:
                    break
                try:
                    obj = json.loads(line.decode('utf-8'))
                except Exception as e:
                    print("Invalid JSON from client:", e)
                    continue

                typ = obj.get('type')
                if typ == 'ping':
                    resp = {'type': 'pong'}
                    resp['ts'] = obj.get('ts')
                    writer.write((json.dumps(resp) + "\n").encode('utf-8'))
                    await writer.drain()
                elif typ == 'identify':
                    username = obj.get('username', 'anonymous')
                    self.clients[writer] = username
                    await self.broadcast({'type': 'player_join', 'username': username})
                elif typ == 'chat':
                    sender = self.clients.get(writer, 'anonymous')
                    msg = obj.get('message', '')
                    await self.broadcast({'type': 'chat', 'sender': sender, 'message': msg})
                elif typ == 'action':
                    # Very simplistic: echo an authoritative zone change as a zone_sync.
                    action = obj.get('action')
                    # Validate / sanitize action here in real server
                    zone_data = {'actor': self.clients.get(writer, 'anonymous'), 'action': action}
                    await self.broadcast({'type': 'zone_sync', 'data': zone_data})
                else:
                    # unknown type -- ignore or log
                    print("Unknown message type:", typ)
        except asyncio.CancelledError:
            pass
        finally:
            # cleanup
            if username:
                await self.broadcast({'type': 'player_leave', 'username': username})
            print(f"Client disconnected: {peer}")
            try:
                del self.clients[writer]
            except KeyError:
                pass
            try:
                writer.close()
                await writer.wait_closed()
            except Exception:
                pass

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', default=12345, type=int)
    args = parser.parse_args()

    srv = Server(args.host, args.port)
    asyncio.run(srv.start())