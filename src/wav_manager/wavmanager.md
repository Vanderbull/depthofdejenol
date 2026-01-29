Important Notes

    QtMultimedia: Ensure the QtMultimedia module is installed and that you add QT += multimedia to your .pro file.

    File Path: Update the wavFolder variable in your main.cpp to the correct directory path where your .wav files are stored.

    Filenames: The playWav() function requires only the filename—exclude both the directory path and the .wav extension (e.g., if the file is named alarm.wav, you should call manager.playWav("alarm")).

    QSoundEffect vs. QMediaPlayer: * QSoundEffect is ideal for short, uncompressed sound effects (such as alerts or button clicks).

        If you need to play longer audio files, streaming content, or compressed formats like MP3, you should use QMediaPlayer instead.
