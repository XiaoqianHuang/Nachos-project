// openfile.cc 
//	Routines to manage an open Nachos file.  As in UNIX, a
//	file must be open before we can read or write to it.
//	Once we're all done, we can close it (in Nachos, by deleting
//	the OpenFile data structure).
//
//	Also as in UNIX, for convenience, we keep the file header in
//	memory while the file is open.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#ifndef FILESYS_STUB

#include "copyright.h"
#include "main.h"
#include "filehdr.h"
#include "openfile.h"
#include "synchdisk.h"

//----------------------------------------------------------------------
// OpenFile::OpenFile
// 	Open a Nachos file for reading and writing.  Bring the file header
//	into memory while the file is open.
//
//	"sector" -- the location on disk of the file header for this file
//----------------------------------------------------------------------

OpenFile::OpenFile(int sector)
{ 
    hdr = new FileHeader();
    hdr->FetchFrom(sector);
    seekPosition = 0;
    hdrSector = sector;
    path = kernel->fileSystem->getFullName(sector);
}

//----------------------------------------------------------------------
// OpenFile::~OpenFile
// 	Close a Nachos file, de-allocating any in-memory data structures.
//----------------------------------------------------------------------

OpenFile::~OpenFile()
{
  delete hdr;
}

//----------------------------------------------------------------------
// OpenFile::Seek
// 	Change the current location within the open file -- the point at
//	which the next Read or Write will start from.
//
//	"position" -- the location within the file for the next Read/Write
//----------------------------------------------------------------------

void
OpenFile::Seek(int position)
{
    seekPosition = position;
}	

//----------------------------------------------------------------------
// OpenFile::Read/Write
// 	Read/write a portion of a file, starting from seekPosition.
//	Return the number of bytes actually written or read, and as a
//	side effect, increment the current position within the file.
//
//	Implemented using the more primitive ReadAt/WriteAt.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//----------------------------------------------------------------------

int
OpenFile::Read(char *into, int numBytes)
{    
   enterReadRegion();
   int result = ReadAt(into, numBytes, seekPosition);
   seekPosition += result;
   leaveReadRegion();
   return result;
}

int
OpenFile::Write(char *into, int numBytes)
{
  enterWriteRegion();

   int result = WriteAt(into, numBytes, seekPosition);
   seekPosition += result;

   leaveWriteRegion();
   return result;
}

//----------------------------------------------------------------------
// OpenFile::ReadAt/WriteAt
// 	Read/write a portion of a file, starting at "position".
//	Return the number of bytes actually written or read, but has
//	no side effects (except that Write modifies the file, of course).
//
//	There is no guarantee the request starts or ends on an even disk sector
//	boundary; however the disk only knows how to read/write a whole disk
//	sector at a time.  Thus:
//
//	For ReadAt:
//	   We read in all of the full or partial sectors that are part of the
//	   request, but we only copy the part we are interested in.
//	For WriteAt:
//	   We must first read in any sectors that will be partially written,
//	   so that we don't overwrite the unmodified portion.  We then copy
//	   in the data that will be modified, and write back all the full
//	   or partial sectors that are part of the request.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//	"position" -- the offset within the file of the first byte to be
//			read/written
//----------------------------------------------------------------------

int
OpenFile::ReadAt(char *into, int numBytes, int position)
{
    //enterReadRegion();

    // -- enter critical region
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    char *buf;

    if ((numBytes <= 0) || (position >= fileLength)) {
      //leaveReadRegion();
      return 0; 				// check request
    }
    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;
    DEBUG(dbgFile, "Reading " << numBytes << " bytes at " << position << " from file of length " << fileLength);

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    // read in all the full and partial sectors that we need
    buf = new char[numSectors * SectorSize];
    for (i = firstSector; i <= lastSector; i++)	
        kernel->synchDisk->ReadSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);

    // copy the part we want
    bcopy(&buf[position - (firstSector * SectorSize)], into, numBytes);
    //leaveReadRegion();
    //  -- leave critical regoin --

    delete [] buf;
    return numBytes;
}

int
OpenFile::WriteAt(char *from, int numBytes, int position)
{
    if (position + numBytes > hdr->FileLength()) {
    OpenFile *mapFile = new OpenFile(0);
    PersistentBitmap *freeMap = new PersistentBitmap(mapFile, NumSectors);
    hdr->Allocate(freeMap, position + numBytes - hdr->FileLength() +1);
    hdr->WriteBack(hdrSector);
    freeMap->WriteBack(mapFile);
    delete freeMap;
    delete mapFile;
    } 

    //enterWriteRegion();
    // -- enter critical region 
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;
    char *buf;


    if ((numBytes <= 0) || (position >= fileLength)) {
      //leaveWriteRegion();
      return 0;				// check request
    }
    if ((position + numBytes) > fileLength)
	numBytes = fileLength - position;
    DEBUG(dbgFile, "Writing " << numBytes << " bytes at " << position << " from file of length " << fileLength);

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    buf = new char[numSectors * SectorSize];

    firstAligned = (position == (firstSector * SectorSize));
    lastAligned = ((position + numBytes) == ((lastSector + 1) * SectorSize));

// read in first and last sector, if they are to be partially modified
    if (!firstAligned)
        ReadAt(buf, SectorSize, firstSector * SectorSize);	
    if (!lastAligned && ((firstSector != lastSector) || firstAligned))
        ReadAt(&buf[(lastSector - firstSector) * SectorSize], 
				SectorSize, lastSector * SectorSize);	

// copy in the bytes we want to change 
    bcopy(from, &buf[position - (firstSector * SectorSize)], numBytes);

// write modified sectors back
    for (i = firstSector; i <= lastSector; i++)	
        kernel->synchDisk->WriteSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);
    //leaveWriteRegion();
    // -- leave critical region --
    delete [] buf;
    return numBytes;
}

//----------------------------------------------------------------------
// OpenFile::Length
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
OpenFile::Length() 
{ 
    return hdr->FileLength(); 
}

char* 
OpenFile::getFullName() {
  return path;
}

int 
OpenFile::getOpenfileId() {
  return openfileId;
}

void 
OpenFile::setOpenfileId(int id) {
  openfileId = id;
}

int
OpenFile::getProtectionBit() {
  return hdr->getProtectionBit();
}

void 
OpenFile::enterReadRegion() {
  release = FALSE;
  string filename = string(path);

  if (kernel->locks->RCLocks->find(filename) == kernel->locks->RCLocks->end()) {
    return;
  }

  if ((*kernel->locks->WRLocks)[filename]->IsHeldByCurrentThread() == FALSE) { // if the lock not held by current thread
    (*kernel->locks->RCLocks)[filename]->Acquire(); // down(RC)
    (*kernel->readCount)[filename]++;
    if ((*kernel->readCount)[filename] == 1) {
      (*kernel->locks->WRLocks)[filename]->Acquire(); // when there is a reader, acquire RW lock
      release = TRUE; // need to release
      DEBUG(dbgSynch, "Reading: RW Lock Aquired.");
    }
    (*kernel->locks->RCLocks)[filename]->Release(); // up(RC)
  }
}

void 
OpenFile::leaveReadRegion() {
  if (release == TRUE) { // only when acquiring a lock at read region will release the lock (WriteAt() will also call ReadAt())
    string filename = string(path);

    if (kernel->locks->RCLocks->find(filename) == kernel->locks->RCLocks->end()) {
      return;
    }

    if ((*kernel->locks->WRLocks)[filename]->IsHeldByCurrentThread() == TRUE) {
      (*kernel->locks->RCLocks)[filename]->Acquire(); // down(RC)
      (*kernel->readCount)[filename]--;
      if ((*kernel->readCount)[filename] == 0) {
        (*kernel->locks->WRLocks)[filename]->Release(); // no reader any more, release RW lock
        DEBUG(dbgSynch, "Reading: RW Lock Released.");

      }
      (*kernel->locks->RCLocks)[filename]->Release(); // up(RC)
    }
  }
}

void 
OpenFile::enterWriteRegion() {
  string filename = string(path);

  if (kernel->locks->RCLocks->find(filename) == kernel->locks->RCLocks->end()) {
    return;
  }

  if ((*kernel->locks->WRLocks)[filename]->IsHeldByCurrentThread() == FALSE) { // if the lock not held by current thread
    (*kernel->locks->WRLocks)[filename]->Acquire(); // acquire RC lock
    DEBUG(dbgSynch, "Writing: RW Lock Aquired.");
  }

}

void 
OpenFile::leaveWriteRegion() {
  string filename = string(path);

  if (kernel->locks->RCLocks->find(filename) == kernel->locks->RCLocks->end()) {
    return;
  }

  if ((*kernel->locks->WRLocks)[filename]->IsHeldByCurrentThread() == TRUE) {
    (*kernel->locks->WRLocks)[filename]->Release(); // release RC lock
    DEBUG(dbgSynch, "Writing: RW Lock Released.");
  }
}

void 
OpenFile::setMode(int m) {
  mode = m;
}

int
OpenFile::getMode() {
  return mode;
}

int 
OpenFile::getSize() {
  return hdr->getdirnum();
}
#endif //FILESYS_STUB
