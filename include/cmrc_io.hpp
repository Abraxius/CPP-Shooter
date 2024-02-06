#pragma once
#ifdef EMBEDDED_MODELS
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <cmrc/cmrc.hpp>
#include <cstring>

// https://assimp.sourceforge.net/lib_html/class_assimp_1_1_i_o_stream.html
class CMRC_IOStream : public Assimp::IOStream {
public:
    size_t FileSize() const override final {
        return file.size();
    }
    size_t Read(void* pBuffer, size_t size, size_t count) override final {

        size_t bytes = size * count; // total requested bytes
        size_t maxBytesLeft = std::max<size_t>(file.cend() - current, 0);
        size_t eligibleBytes = std::min<size_t>(bytes, maxBytesLeft);
        size_t nCounts = eligibleBytes / size; // calc number of valid objects
        if (nCounts == 0 || count == 0) return 0;

        // final number of bytes read = valid objects * their size
        eligibleBytes = nCounts * size;
        std::memcpy(pBuffer, current, eligibleBytes);
        current += eligibleBytes;

        // return number of objects that were read
        return nCounts;
    }
    aiReturn Seek(size_t offset, aiOrigin origin) override final {
        switch(origin) {
            case aiOrigin_END: 
                current = file.cend() + offset;
                if (current < file.cbegin()) return aiReturn_FAILURE;
                else return aiReturn_SUCCESS;

            case aiOrigin_CUR: 
                current = current + offset; 
                if (current >= file.cend()) return aiReturn_FAILURE;
                else return aiReturn_SUCCESS;

            case aiOrigin_SET:
                current = file.cbegin() + offset;
                if (current >= file.cend()) return aiReturn_FAILURE;
                else return aiReturn_SUCCESS;

            default: 
                std::cerr << "invalid origin\n"; 
                return aiReturn_FAILURE;
        }
    }
    size_t Tell() const override final {
        return current - file.cbegin();
    }
    void Flush() override final {
        return;
    }
    size_t Write(const void *pvBuffer, size_t pSize, size_t pCount) override final { 
        return 0; 
    }
protected:
    CMRC_IOStream() = default;

private:
    friend class CMRC_IOSystem;
    cmrc::file file;
    cmrc::file::iterator current;
};

// https://assimp.sourceforge.net/lib_html/class_assimp_1_1_i_o_system.html
class CMRC_IOSystem : public Assimp::IOSystem {
public:
    CMRC_IOSystem() : fs(cmrc::models::get_filesystem()) {}
    ~CMRC_IOSystem() = default;

    // override all the virtual functions of Assimp::IOSystem
    bool ComparePaths(const char* pFirst, const char* pSecond) const override final {
        std::string first(pFirst);
        return first.compare(pSecond);
    }
    bool Exists(const char* pFile) const override final {
        return fs.exists(pFile);
    }
    char getOsSeparator() const override final {
        return '/';
    }
    void Close(Assimp::IOStream* pIOStream) override final {
        delete pIOStream;
    }
    Assimp::IOStream* Open(const char *pFile, const char *pMode = "rb") override final {
        CMRC_IOStream* pIOStream = new CMRC_IOStream();
        if (fs.exists(pFile)) {
            pIOStream->file = fs.open(pFile);
            pIOStream->current = pIOStream->file.cbegin();
            return pIOStream;
        }
        else {
            std::cerr << "Unable to open model: " << pFile << std::endl;
            return nullptr;
        }
    }

private:
    cmrc::embedded_filesystem fs;
};
#endif