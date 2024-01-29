// This class is needed to convert a C-style FILE* to a C++-style stream to make
// popen compatible with the large portions of SANA that use C++ to read files
#include <streambuf>

#ifndef STDIOBUF_HPP
#define STDIOBUF_HPP

#define BUFFER_SIZE 10240

class stdiobuf : public std::streambuf
{
private:
    FILE* file;
    char buffer[BUFFER_SIZE];
    bool isPiped = false;
public:
    stdiobuf(FILE* file, bool piped): file(file), isPiped(piped) {}
    ~stdiobuf() {
        if (this->file) {
            if(isPiped) pclose(file);
            else fclose(this->file); 
        }
    }
    stdiobuf(const stdiobuf& other) {
        if(&other != this) {
            this->file = other.file;
            this->isPiped = other.isPiped;
            for(int i = 0; i < BUFFER_SIZE; i++)
                this->buffer[i] = other.buffer[i];
        }
    }

    int underflow() {
        if (this->gptr() == this->egptr() && this->file) {
            size_t size = fread(this->buffer, 1, BUFFER_SIZE, this->file);
            if(0 < size)
                this->setg(this->buffer, this->buffer, this->buffer + size);
        }
        return this->gptr() == this->egptr()
            ? traits_type::eof()
            : traits_type::to_int_type(*this->gptr());
    }
    
};
#endif
