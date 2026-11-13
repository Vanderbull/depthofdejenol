#pragma once

#include <QFile>
#include <QDataStream>
#include <QString>
#include <QBuffer> 
#include <stdexcept>
#include <stdint.h>
#include <QDebug> 

// Define standard types for clarity
typedef int32_t int32_t;
typedef uint32_t uint32_t;
typedef int64_t int64_t;
typedef uint64_t uint64_t;
typedef uint8_t uint8_t; // Added for single-byte reads

/**
 * @brief Template class for reading fixed-length records from a file using QDataStream.
 * @tparam RECORD_LENGTH The size of the record in bytes.
 */
template<size_t RECORD_LENGTH>
class RecordReader {
public:
    RecordReader(const QString& filename) : 
        file(filename), 
        s(&file), 
        readRecord(false) 
    {
        if (!file.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Could not open file for reading: " + filename.toStdString());
        }
        // Set to LittleEndian as per file analysis
        s.setByteOrder(QDataStream::LittleEndian);
        s.setVersion(QDataStream::Qt_6_0); 
    }

    // --- Core Read and Seek Methods ---

    /**
     * @brief Reads the next fixed-size record from the file into the internal buffer.
     */
    void read() {
        QByteArray data = file.read(RECORD_LENGTH);
        if (data.size() != (int)RECORD_LENGTH) {
            if (file.atEnd() && data.isEmpty()) {
                throw std::runtime_error("Unexpected end of file or incomplete record read.");
            }
            throw std::runtime_error("Incomplete record read: Expected " + std::to_string(RECORD_LENGTH) + " bytes, got " + std::to_string(data.size()));
        }
        
        // Reset the buffer for the new data
        buffer.close();
        buffer.setData(data); // Use setData instead of setBuffer for a clean copy
        
        if (!buffer.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Failed to open QBuffer for read.");
        }
        
        // CRITICAL: Point QDataStream to the new buffer BEFORE reading values
        s.setDevice(&buffer); 
        
        readRecord = true;
    }

    void seek(qint64 offset) {
        if (!file.seek(offset)) {
            throw std::runtime_error("Failed to seek file.");
        }
        readRecord = false;
    }

    // --- Public Getter Template (Delegates to private overloads) ---
    /**
     * @brief Reads a value of type T from the current record buffer.
     */
    template<typename T>
    T& get(T& var) {
        if (!readRecord) throw "Tried to get before reading next record";
        return readValue(var); // Calls the private readValue helper
    }

    // --- Helper Getters for Primitive Types (Convenience) ---

    uint8_t getByte() {
        uint8_t var;
        get(var);
        return var;
    }

    uint16_t getWord() { 
        uint16_t var; 
        get(var); 
        return var; 
    }
    
    uint32_t getDword() { 
        uint32_t var; 
        get(var); 
        return var; 
    }

    float getFloat() {
        float var;
        get(var);
        return var;
    }
    
    int64_t getCurrency() {
        int64_t var = 0; 
        get(var); 
        return var;
    }
    
    int64_t getIntCurrency() {
        int64_t var = 0; 
        get(var);
        return var / 10000; 
    }
    
    // --- String Getters (FIXED LENGTH) ---
    /**
     * @brief Reads a fixed-length string from the current record buffer.
     * @param len The exact byte length of the string field.
     * * NOTE: This uses the QBuffer's read() which advances the QBuffer's pointer.
     */
    QString getString(size_t len = 0) {
        if (!readRecord) throw "Tried to get before reading next record";
        
        QByteArray arr;
        if (len == 0) {
            // Fall back to standard length-prefixed Qt read
            this->s >> arr;
        } else {
            // Reads fixed-length from the internal QBuffer device
            // This is the source of conflict, but necessary for fixed-length strings.
            arr = buffer.read(len); 
        }
        
        return QString::fromLatin1(arr);
    }

private:
    // ----------------------------------------------------------------------
    // Private Overloaded Helper Functions (ALL using QDataStream 's')
    // ----------------------------------------------------------------------
    
    // 1. Generic readValue (Used for all types QDataStream supports directly)
    template<typename T>
    T& readValue(T& var) {
        this->s >> var;
        return var;
    }

    // 2. Overload for int64_t
    int64_t& readValue(int64_t& var) {
        qint64 temp; 
        this->s >> temp; 
        var = temp;      
        return var;
    }

    // 3. Overload for uint64_t
    uint64_t& readValue(uint64_t& var) {
        quint64 temp; 
        this->s >> temp; 
        var = temp;
        return var;
    }
    
private:
    QFile file;
    QDataStream s;
    QBuffer buffer; 
    bool readRecord;
};
