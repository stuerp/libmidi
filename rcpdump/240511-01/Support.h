
/** $VER: Support.h (2024.05.10) P. Stuer **/

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>

#include <algorithm>

#include "MIDI.h"

uint32_t MulDivCeil(uint32_t val, uint32_t mul, uint32_t div);
uint32_t MulDivRound(uint32_t val, uint32_t mul, uint32_t div);
uint16_t ReadLE16(const uint8_t * data);
uint32_t ReadLE32(const uint8_t * data);

uint16_t GetTrimmedLength(const char * data, uint16_t size, char trimChar, bool leaveLast);
uint32_t BPM2Ticks(uint16_t bpm, uint8_t scale);

const char * GetFileName(const char * filePath);
const char * GetFileExtension(const char * fileName);

struct buffer_t
{
    uint8_t * Data;
    uint32_t Size;

    buffer_t() : Data(), Size() { }

    buffer_t(uint32_t size)
    {
        Data = (uint8_t *) ::malloc(size);
        Size = size;
    }

    buffer_t & operator =(const buffer_t & other)
    {
        Copy(other.Data, other.Size);

        return *this;
    }

    virtual ~buffer_t()
    {
        Reset();
    }

    void Copy(const void * data, size_t size)
    {
        if ((data == nullptr) || (size == 0))
            return;

        if (Data != nullptr)
            ::free(Data);

        Data = (uint8_t *) ::malloc(size);
        Size = (uint32_t) size;

        if (Data != nullptr)
            ::memcpy(Data, data, Size);
    }

    void Grow(uint32_t size)
    {
        void * NewData = ::realloc(Data, size);

        if (NewData != nullptr)
        {
            Data = (uint8_t *) NewData;
            Size = size;
        }
    }

    bool ReadFile(const char * filePath)
    {
        FILE * hFile;

        hFile = ::fopen(filePath, "rb");

        if (hFile == NULL)
            return false;

        ::fseek(hFile, 0, SEEK_END);

        Size = std::min(::ftell(hFile), (long) 0x100000);

        ::fseek(hFile, 0, SEEK_SET);

        Data = (uint8_t *) ::malloc(Size);

        if (Data != nullptr)
            ::fread(Data, 1, Size, hFile);

        ::fclose(hFile);

        return true;
    }

    bool WriteFile(const char * filePath) const
    {
        FILE * hFile = ::fopen(filePath, "wb");

        if (hFile == NULL)
            return false;

        ::fwrite(Data, 1, Size, hFile);
        ::fclose(hFile);

        return true;
    }

 private:
    void Reset()
    {
        if (Data != nullptr)
        {
            ::free(Data);

            Data = nullptr;
            Size = 0;
        }
    }
};
