// This class is needed to convert a C-style FILE* to a C++-style stream to make
// popen compatible with the large portions of SANA that use C++ to read files
#include <streambuf>

#ifndef STDIOBUF_HPP
#define STDIOBUF_HPP

class stdiobuf : public std::streambuf
{
private:
    FILE* d_file;
    char  d_buffer[10240];
    bool isPiped = false;
public:
    stdiobuf(FILE* file, bool piped): d_file(file), isPiped(piped) {}
    ~stdiobuf() {
        if (this->d_file) {
            if(isPiped)
                pclose(d_file);
            else
                fclose(this->d_file); 
        } 
    }
    stdiobuf(const stdiobuf& other)
    {
        if(&other != this)
        {
            this->d_file = other.d_file;
            this->isPiped = other.isPiped;
            for(int i = 0; i < 10240; i++)
                this->d_buffer[i] = other.d_buffer[i];
        }
    }

    int underflow() {
        if (this->gptr() == this->egptr() && this->d_file) {
            size_t size = fread(this->d_buffer, 10240, 1, this->d_file);
            this->setg(this->d_buffer, this->d_buffer, this->d_buffer + size);
        }
        return this->gptr() == this->egptr()
            ? traits_type::eof()
            : traits_type::to_int_type(*this->gptr());
    }
};
#endif
