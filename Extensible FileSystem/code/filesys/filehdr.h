// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "pbitmap.h"

#define NumDirect 	((SectorSize - 8 * sizeof(int)) / sizeof(int)) // 22?
//#define MaxFileSize 	(NumDirect * SectorSize) // 4 kb for now
#define MaxFileSize (SectorSize * SectorsPerTrack * NumTracks -  (NumInode + 2) * SectorSize);

#define NumInode 96
#define NumInDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int)) 
#define MaxBlockSize 	(NumInDirect * SectorSize) 

class FileBlock {
public:
  FileBlock(/*int index*/);
  ~FileBlock();
  int Allocate(PersistentBitmap *bitMap, int fileSize);// Initialize a file header, 
          //  including allocating space 
          //  on disk for the file data
  void Deallocate(PersistentBitmap *bitMap);  // De-allocate this file's 
          //  data blocks

  void FetchFrom(int sectorNumber); 	// Initialize file header from disk
  void WriteBack(int sectorNumber); 	// Write modifications to file header
        //  back to disk

  int ByteToSector(int offset);	// Convert a byte offset into the file
  //      // to the disk sector containing
  //      // the byte

  void Print();			// Print the contents of the file.
  void setNextBlock(int block);
  int FileLength();
  int getNextBlock();
  int Expand(PersistentBitmap *freeMap, int expendedSize);

private:
  //int index;
  int dataSectors[NumInDirect];		// Disk sector numbers for each data  //sector[30], making file max size = 4kb
        // block in the file
  int nextBlock;
  int numBytes;
};

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    FileHeader();
    bool Allocate(PersistentBitmap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(PersistentBitmap *bitMap);  // De-allocate this file's 
						//  data blocks

   // int Expend(PersistentBitmap *bitMap, int expendedSize);

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    void setProtectionBit(int protection);
    int getProtectionBit();
    void setParentSector(int sector);
    int getParentSector();
    bool isDir();
    void setDir();
    int getdirnum();
    void setdirnum(int n);

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    //int dataSectors[NumDirect];		// Disk sector numbers for each data  //sector[30], making file max size = 4kb
					// block in the file
    int protectionBit; // protection bit
    int firstBlock;
    int lastBlock; // used for speedup expend
    bool dirFlag;
    int parentSector; // speedup searching
    int dataSectors[NumDirect]; 
    int dirnum;
};

#endif // FILEHDR_H
