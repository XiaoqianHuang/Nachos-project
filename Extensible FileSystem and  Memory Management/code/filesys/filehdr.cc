// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"


int
FileBlock::Allocate(PersistentBitmap *freeMap, int fileSize)
{
  DEBUG(dbgFile, " - Allocate at file block, size: " << fileSize);
  numBytes = fileSize;
  int numSectors = divRoundUp(fileSize, SectorSize);
  if (freeMap->NumClearRange(NumInode + 2, NumSectors) < numSectors) {
    DEBUG(dbgFile, " - Not enough space: " << fileSize);
    return -1;		// not enough space
  }

  for (int i = 0; i < numSectors; i++) {
    dataSectors[i] = freeMap->FindAndSetRange(NumInode + 2, NumSectors);
    DEBUG(dbgFile, " - Allocated sector: " << dataSectors[i]);
    // since we checked that there was enough free space,
    // we expect this to succeed
    ASSERT(dataSectors[i] >= 0);
  }

  return fileSize;
}

void
FileBlock::Deallocate(PersistentBitmap *freeMap)
{
  int numSectors = divRoundUp(numBytes, SectorSize);
  for (int i = 0; i < numSectors; i++) {
    ASSERT(freeMap->Test((int)dataSectors[i]));  // ought to be marked!
    freeMap->Clear((int)dataSectors[i]);
  }
}


void
FileBlock::FetchFrom(int sector)
{
  kernel->synchDisk->ReadSector(sector, (char *)this);
}

void
FileBlock::WriteBack(int sector)
{
  kernel->synchDisk->WriteSector(sector, (char *)this);
}

int
FileBlock::ByteToSector(int offset)
{
  return(dataSectors[offset / SectorSize]);
}


int
FileBlock::FileLength()
{
  return numBytes;
}

FileBlock::FileBlock(/*int i*/) {
  for (int i = 0; i < NumInDirect; ++i) {
    dataSectors[i] = -1;
  }
  //index = i;
  nextBlock = NULL;
}

FileBlock::~FileBlock() { //gc
}

void
FileBlock::setNextBlock(int block) {
  nextBlock = block;
}

int
FileBlock::getNextBlock() {
  return nextBlock;
}

int
FileBlock::Expand(PersistentBitmap *freeMap, int expendedSize) {
  int requiredByte = expendedSize;
  int remain = 0;
  int index = numBytes / SectorSize; // when numBytes % SectorSize == 0, full or empty?
  if (numBytes % SectorSize == 0 && dataSectors[index] != -1) { // full
    remain = 0;
  }
  else {
    remain = SectorSize - (numBytes % SectorSize);
  }

  requiredByte = requiredByte - remain;

  if (remain == SectorSize) { // block not assigned yet
    if (freeMap->NumClearRange(NumInode + 2, NumSectors) < 1) {
      DEBUG(dbgFile, " - Not enough space: " << 1);
      return -1;		// not enough space
    }

    dataSectors[index] = freeMap->FindAndSetRange(NumInode + 2, NumSectors);
  }

  DEBUG(dbgFile, " - Expand at file block, size: " << expendedSize);
  DEBUG(dbgFile, " - remain: " << remain);

  int requiredSectors = divRoundUp(requiredByte, SectorSize);
  int allocatedSectors = 0;
  int allocatedSize = remain;
  if (freeMap->NumClearRange(NumInode + 2, NumSectors) < requiredSectors)
    return -1;		// not enough space

  for (int i = index + 1; i < NumInDirect; i++) {
    if (allocatedSize >= requiredByte) {
      break;
    }
    if (dataSectors[i] == -1) { // empty
      dataSectors[i] = freeMap->FindAndSetRange(NumInode + 2, NumSectors);
      allocatedSectors++;
      allocatedSize = allocatedSize + SectorSize;
      DEBUG(dbgFile, " - Expand at file block, size: " << allocatedSize);
      DEBUG(dbgFile, "   Expand at file block, sector: " << dataSectors[i]);
    }
  }

  numBytes = numBytes + allocatedSize;
  ASSERT(numBytes <= NumInDirect * SectorSize);
  return allocatedSize;
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the size of file to allocate
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    DEBUG(dbgFile, "Allocating, size: " << fileSize);
    int requiredSize = fileSize;
    int reqSector  = divRoundUp(fileSize, SectorSize);
    int numBlocks = divRoundUp(reqSector, NumInDirect); // calculate the indirect block required to allocate

    if (fileSize == 0) {
      firstBlock = -1;
      lastBlock = -1;
      return TRUE;
    }

    if (freeMap->NumClearRange(NumInode + 2, NumSectors) - numBlocks < numSectors)
	return FALSE;		// not enough space

    FileBlock *newBlock;
    FileBlock *preBlock = NULL; //preBlock
    int allocateSize = MaxBlockSize;
    int allocatedSize = 0;
    int preSector = -1;
    int i = 0;
    DEBUG(dbgFile, "[First sector] " << firstBlock << "[Last sector] " << lastBlock << "[Previous numbyte] " << numBytes << "[Required]: " << requiredSize << "[Allocated]: " << allocatedSize);

    if (lastBlock != -1) { // has data before, link the old
      preSector = lastBlock;
      newBlock = new FileBlock();
      newBlock->FetchFrom(lastBlock); // fetch last block.

      allocateSize = MaxBlockSize - newBlock->FileLength(); // assgining remaining space
      allocatedSize = newBlock->Expand(freeMap, allocateSize);
      requiredSize = requiredSize - allocatedSize; // update require size
      newBlock->WriteBack(lastBlock);
      preBlock = newBlock;
    }

    while(requiredSize > allocatedSize) {

      //print info , why 10240 fail?
      DEBUG(dbgFile, "[required]: " << requiredSize);
      DEBUG(dbgFile, "[allocated]: " << allocatedSize);

      newBlock = new FileBlock(); // new block
      int allocatedSector = freeMap->FindAndSetRange(NumInode + 2, NumSectors);
      DEBUG(dbgFile, "Allocating sector: " << allocatedSector);
      if ((requiredSize - allocatedSize) >= MaxBlockSize) {
        allocateSize = MaxBlockSize;
      }
      else {
        allocateSize = requiredSize - allocatedSize;
      }

      DEBUG(dbgFile, "[allocated after]: " << allocateSize);

      if (newBlock->Allocate(freeMap, allocateSize) == -1) {
        DEBUG(dbgFile, "Allocating fail");
        return FALSE; // allocation fail
      }
      if (firstBlock == -1) { // no data before
        firstBlock = allocatedSector;
      }
      else {
        preBlock->setNextBlock(allocatedSector); // link previous block to current block
        preBlock->WriteBack(preSector);
      }
      newBlock->WriteBack(allocatedSector); // flush the disk at the allocated sector
      allocatedSize = allocatedSize + allocateSize;

      if (preBlock != NULL) {
        delete preBlock; // gc
      }

      preBlock = newBlock;
      preSector = allocatedSector;
      if (allocatedSize >= requiredSize) {
        lastBlock = allocatedSector;
        delete newBlock;
      }
      i++;
    }

    numBytes = numBytes + allocatedSize;
    numSectors = numSectors + numBlocks; // update new blocks
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
  DEBUG(dbgFile, "Deallocating..");
  int numBlocks = divRoundUp(numSectors, NumInDirect);
  int blockToDeall = firstBlock;
  FileBlock *block;
  for (int i = 0; i < numBlocks; i++) {
    block = new FileBlock();
    block->FetchFrom(blockToDeall); // fetch the block back
    block->Deallocate(freeMap);
    ASSERT(freeMap->Test(blockToDeall));
    freeMap->Clear(blockToDeall);
    blockToDeall = block->getNextBlock(); // fetch next block
    delete block;
  }
  firstBlock = -1;
  lastBlock = -1;
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
  int index = offset / MaxBlockSize;
  int blocknum = (offset % MaxBlockSize) / SectorSize;
  FileBlock *block;
  int blockFetched = firstBlock;

  for (int i = 0; i < numSectors; i++) {
    block = new FileBlock();
    block->FetchFrom(blockFetched);
    if (i == index) { // corresponding FileBlock
      int result = block->ByteToSector(offset % MaxBlockSize);
      delete block;

      DEBUG(dbgFile, " => index:" << index << " is at block no. " << blocknum);
      DEBUG(dbgFile, " ** The offset:" << offset << " is at block " << result);
      return result;
    }
    blockFetched = block->getNextBlock(); // get next block sector number
    delete block;
  }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

FileHeader::FileHeader() {
  //for (int i = 0; i < NumDirect; ++i) {
  //  dataSectors[i] = -1;
  //}
  numBytes = 0;
  numSectors = 0; // initial file size to 0
  protectionBit = 7; // initial to r w x
  for (int i = 0; i < NumDirect; i++) {
    dataSectors[i] = -1;
  }
  parentSector = 1;
  dirFlag = FALSE;
  lastBlock = -1;
  firstBlock = -1;

}

void 
FileHeader::setProtectionBit(int protection) {
  protectionBit = protection;
}

int 
FileHeader::getProtectionBit() {
  return protectionBit;
}


void 
FileHeader::setParentSector(int sector) {
  parentSector = sector;
}

int 
FileHeader::getParentSector() {
  return parentSector;
  }

bool 
FileHeader::isDir(){
  return dirFlag;
}

void
FileHeader::setDir() {
  dirFlag = TRUE;
}

int 
FileHeader::getdirnum() {
  return dirnum;
}

void 
FileHeader::setdirnum(int n) {
  dirnum = n;
}