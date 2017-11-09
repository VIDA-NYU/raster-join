#ifndef BUFFERED_FILE
#define BUFFERED_FILE

#include <QDebug>
#include <fstream>
#include <iostream>

#include "Common.h"
using namespace std;

/*
     * Class for reading and writing on file
     * supports large disk files
     * uses internal buffer for fast access
     * Does not allocates or deallocates memory.
     * Allocating and Deallocating is responsibility of the caller
*/
/* Methods
 *
    BufferedFile();
    ~BufferedFile();

    void open(string filename);
    void create(string filename);
    void maketemporary();

    void close();
    void rewindForReading();

    void seekEnd(quint64 offset);

    void seek(quint64 offset);
    void read(char* buffer, quint32 size);
    void write(char* buffer, quint32 size);
*/

class BufferedFile
{
public:

    fstream file;
    string filename;
    char* buffer;
    bool eof;
    bool temporary;

    BufferedFile()
    {
        eof = true;
        temporary = false;
        buffer = NULL;
    }

    ~BufferedFile()
    {
        close();
        if (temporary)
        {
            if (remove(filename.c_str())!=0)
            {
#ifdef FATAL
                cout << "Cannot Delete Temporary File: " << filename << "\n";
#endif
            }
        }
    }

    void open(string filename)
    {
        try
        {
            this->filename = filename;
            close();
            eof=false;
            temporary=false;
            buffer = new char[FILE_BUFFER_SIZE];

            file.open(filename.c_str(), ios_base::in | ios_base::binary);
            file.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);

            if (!file.good()) throw 1;
        }
        catch(...)
        {
            eof = true;
#ifdef FATAL
            cout << "Cannot Open file: " << filename << "\n";
#endif
        }
    }

    void filesize()
    {
    	  streampos begin,end;
    	  begin = file.tellg();
    	  file.seekg (0, ios::end);
    	  end = file.tellg();
    	  cout << "size is: " << end-begin << " bytes.\n";
    }

    void create(string filename)
    {
        try
        {
            this->filename = filename;
            close();
            eof = false;
            temporary = false;
            buffer = new char[FILE_BUFFER_SIZE];

            file.open(filename.c_str(), std::ios_base::out | ios_base::in | std::ios_base::binary | std::ios_base::trunc);
            //file.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

            file.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);

            if (!file.good()) throw 1;
        }
        catch(...)
        {
            eof = true;
#ifdef FATAL
            cout << "Cannot Create file: " << filename << "\n";
#endif
        }
    }

    void maketemporary()
    {
#ifdef WIN32
        char tmpName[11] = "tempXXXXXX";
        char * tmp_name_ptr = tmpnam(tmpName);
        if (tmp_name_ptr[0] == '\\')
            filename = string(tmp_name_ptr + 1);
        else
            filename = string(tmp_name_ptr);
#else
        char tmpName[11] = "tempXXXXXX";
        if (mkstemp(tmpName) == 0)
#ifdef FATAL
            cout << "Cannot Make Temporary filename: \n";
#endif
        filename = tmpName;
#endif
        create (filename);
        temporary = true;
    }

    void  close()
    {
        file.flush();
        
        eof=true;
        file.clear();
        if (file.is_open())
            file.close();
        delete[] buffer;
        buffer  = NULL;
    }


    void rewindForReading()
    {
        close();
        if (temporary)
        {
            open(filename);
            temporary = true;
        }
        else
            open(filename);
    }

    /*
     * not fixed for large files
     */
    void seekEnd(quint64 offset)
    {
        try
        {
            eof = false;
            file.clear();
            file.seekg(offset*-1, ios_base::end);
            if (!file.good()) throw 1;
        }
        catch(...)
        {
            eof = true;
#ifdef FATAL
            cout << "Cannot Seek file" << filename << " to:" << offset << "\n";
#endif
        }
    }


    inline void seek(quint64 offset)
    {
        try
        {
            eof = false;
            file.clear();
#ifdef WIN32
            file.seekg(0, std::ios_base::beg);
            uint32_t n4GBSeeks = offset / 0xFFFFFFFF;
            for(uint32_t i = 0; i < n4GBSeeks; i++)
                file.seekg(0xFFFFFFFF, ios_base::cur);
            file.seekg(offset % 0xFFFFFFFF, ios_base::cur);
#else
            file.seekg(offset, ios_base::beg);
#endif
            if (!file.good()) throw 1;
        }
        catch(...)
        {
            eof = true;
#ifdef FATAL
            cout << "Cannot Seek file" << filename << " to:" << offset << "\n";
#endif
        }
    }

    inline const char* readFile(string filename)
    {
    	open(filename);
    	if (file) {
    		file.seekg (0, file.end);
            int64_t length = file.tellg();
    		file.seekg (0, file.beg);

    		char * buffer = new char [length];

            qDebug() << "Reading " << length << " characters... " << endl;
    		// read data as a block:
            file.read (buffer,length);

    		if (file)
                qDebug() << "all characters read successfully." << endl;
    		else
                qDebug() << "error: only " << file.gcount() << " could be read" << endl;
    		file.close();
    		return (const char*) buffer;
    		// ...buffer contains the entire file...
    	}
    }


    inline void read(char* buffer, quint32 size)
    {
#ifdef FATAL
        if (eof)
            cout << "Reading file after eof : " << filename << "\n";
#endif
        try
        {
            file.read(buffer,size);
            if (!file.good()) throw 1;
            eof = file.eof();
        }
        catch(...)
        {
            eof= true;
        }
    }

    inline void write(char *buffer, quint32 size)
    {
        try
        {
            file.write(buffer, size);
            if (!file.good()) throw 1;
            eof = file.eof();
        }
        catch(...)
        {
            eof= true;
#ifdef FATAL
            cout << "Problem writing buffer to disk file: " << filename << "\n";
#endif
        }
    }
};


#endif // BUFFERED_FILE

