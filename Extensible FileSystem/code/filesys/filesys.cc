// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#ifndef FILESYS_STUB

#include "copyright.h"
#include "debug.h"
#include "disk.h"
#include "pbitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"
#include <string>
#include "synch.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1 // root

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

static char *protectionName[] = { "- - x", "- w -", "- w x",
      "r - -", "r - x",
      "r w -", "r w x" };

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG(dbgFile, "Initializing the file system.");
    currentDirSector = DirectorySector;

    if (format) {
        PersistentBitmap *freeMap = new PersistentBitmap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

        DEBUG(dbgFile, "Formatting the file system.");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

  char *name = new char[4];

  dirHdr->setParentSector(1029);
  dirHdr->setDir(); // mark the dir flag
  dirHdr->setdirnum(NumDirEntries);

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

   DEBUG(dbgFile, "Writing headers back to disk.");
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.
   
  freeMapFile = new(nothrow) OpenFile(FreeMapSector);
  directoryFile = new(nothrow)  OpenFile(DirectorySector);
  
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG(dbgFile, "Writing bitmap and directory back to disk.");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk

  directory->Add(".", DirectorySector); // add entry to point itself.
	directory->WriteBack(directoryFile);

	if (debug->IsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();
        }
        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, int initialSize, int currentDirSector, int protection)
{
  if (name == NULL) {
    return FALSE;
  }

    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *hdr;
    OpenFile *openFile;
    int sector;
    bool success;

    DEBUG(dbgFile, "Creating file " << name << " size " << initialSize);

    if (protection > 7 || protection < 1) {
      cout << "Fail, invalid protection bit.\n";
      return FALSE;
    }

    if (currentDirSector == -1) {
      currentDirSector = DirectorySector;
    }

    // check the protection bit of dir?
    int parentSector = parsePath(&name, currentDirSector);
    openFile = new OpenFile(parentSector);
    if (canWrite(openFile->getProtectionBit()) == FALSE) {
      cout << "Privilege Denied: dir - " << openFile->getProtectionBit() << "\n";
    }

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(openFile);

    if (directory->Find(name) != -1) { // file is already in directory
      success = FALSE;			
      cout << "The file - " << name << " exist at current dir - " << getFullName(currentDirSector) << "\n";
      directory->Print();
    }
    else {	
        freeMap = new PersistentBitmap(freeMapFile,NumSectors);
        sector = freeMap->FindAndSetRange(2, 2 + NumInode);	// find a sector to hold the file header (sector 2 - 47)
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
      else if (!directory->Add(name, sector)) {
        cout << "Fail, the current directory is full! \n";
        success = FALSE;	// no space in directory
      }
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, initialSize))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;

        hdr->setParentSector(parentSector); // save parent sector
        hdr->setProtectionBit(protection); // set protection bit
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(openFile);
    	    	freeMap->WriteBack(freeMapFile);
	    }
            delete hdr;
	}
        delete freeMap;
    }
    delete directory;
    delete openFile;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name)
{ 
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG(dbgFile, "Opening file " << name);
    sector = parsePath(&name, currentDirSector); // accquire the reqiured dir sector

    if (sector == -1) {
      DEBUG(dbgFile, "cannot find sector " << sector);
    }

    openFile = new OpenFile(sector); // fetch dir file
    directory->FetchFrom(openFile);
    delete openFile;
    openFile = NULL;

    sector = directory->Find(name);
    if (sector > -1) { // find the file
      openFile = new OpenFile(sector);
    }

    delete directory;
    return openFile;

 // // single dir
 //   directory->FetchFrom(directoryFile);
 //   sector = directory->Find(name); 
 //   if (sector >= 0) 		
	//openFile = new OpenFile(sector);	// name was found in directory 
 //   delete directory;
 //   return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name) // make sure no process access to the file any more
{ 
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    DEBUG(dbgFile, "Removing file " << name);
    int dirSector = parsePath(&name, currentDirSector); // accquire the reqiured dir sector

    OpenFile *openFile = new OpenFile(dirSector); // fetch dir file
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(openFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new PersistentBitmap(freeMapFile,NumSectors);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(openFile);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    delete openFile;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    openFile = new OpenFile(currentDirSector);
    directory->FetchFrom(openFile);
    DEBUG(dbgFile, "Listing dir " << openFile->getFullName());
    DirectoryEntry* table = directory->getEntry();
    for (int i = 0; i < NumDirEntries; i++) {
      if (table[i].inUse) {
        FileHeader *hdr = new FileHeader();
        hdr->FetchFrom(table[i].sector);
        printf("%-50s%s\n", table[i].name, protectionName[hdr->getProtectionBit() - 1]);
        delete hdr;
      }
    }
    
    delete openFile;
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    PersistentBitmap *freeMap = new PersistentBitmap(freeMapFile,NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    //bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    //dirHdr->Print();

    printf("Current file header:\n");
    OpenFile *openFile = NULL;
    dirHdr->FetchFrom(currentDirSector);
    //dirHdr->Print();
    openFile = new OpenFile(currentDirSector);

    freeMap->Print();

    directory->FetchFrom(openFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
    delete openFile;
} 

int 
FileSystem::ChangeDir(char **name, int currentDir){
  Directory *directory = new Directory(NumDirEntries);
  OpenFile *openFile = NULL;
  int sector;

  DEBUG(dbgFile, "Change to dir " << *name);
  sector = parsePath(name, currentDir); // accquire the reqiured dir sector
  if (sector == -1) { // cannot open
    cout << "Error path! \n";
    return -1;
  }

  openFile = new OpenFile(sector);
  directory->FetchFrom(openFile);
  sector = directory->Find(*name);
  if (sector == -1) {
    cout << "Error path! \n";
    return -1;
  }

  if (canRead(openFile->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: dir - " << openFile->getProtectionBit() << "\n";
    return -1;
  }

  FileHeader *hdr = new FileHeader();
  hdr->FetchFrom(sector); // target hdr
  if (hdr->isDir() == FALSE) {
    cout << "Not a dir! \n";
    return -1;
  }
  currentDirSector = sector;

  cout << "Now in dir: " << getFullName(sector) << "\n";
  *name = getFullName(sector);

  delete directory;
  delete openFile;
  delete hdr;
  return sector;
  // return current dir sector
}

bool 
FileSystem::MakeDir(char **name, int currentDir) { // name cannot contains "/"
  Directory *directory = new Directory(NumDirEntries);
  OpenFile *openFile = NULL;
  FileHeader *dirHdr;
  bool success;
  int pSector;

  pSector = parsePath(name, currentDir);
  if (pSector < 0) {
    DEBUG(dbgFile, "cannot find path: " << *name);
    return FALSE;
  }

  openFile = new OpenFile(pSector); // fetch dir file
  directory->FetchFrom(openFile);

  if (canWrite(openFile->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: dir - " << openFile->getProtectionBit() << "\n";
    return -1;
  }

  if (directory->Find(*name) != -1) {
    cout << "Fail, the directory " << *name << " has already existed.\n";
    success = FALSE; // dir already in directory
  }
  else {
    PersistentBitmap *freeMap = new PersistentBitmap(freeMapFile, NumSectors);
    int hdrSector = freeMap->FindAndSetRange(2, 2 + NumInode);
    if (hdrSector == -1) {
      success = FALSE;
    }
    else if (directory->Add(*name, hdrSector) == FALSE) {
      cout << "Fail, the current directory is full!\n";
      success = FALSE;
    }
    else {
      dirHdr = new FileHeader();
      if (dirHdr->Allocate(freeMap, DirectoryFileSize) == FALSE) {
        success = FALSE;
      }
      else {
        success = TRUE;
        dirHdr->setProtectionBit(6); // default: r w -
        dirHdr->setParentSector(pSector); // record parent sector
        dirHdr->setDir(); // mark as dir
        dirHdr->setdirnum(NumDirEntries);
        dirHdr->WriteBack(hdrSector); // save hdr
        directory->WriteBack(openFile); // save parent dir

        freeMap->WriteBack(freeMapFile);// save free map;
        Directory *newDir = new Directory(NumDirEntries);
        OpenFile *newDirFile = new OpenFile(hdrSector);

        newDir->Add(".", hdrSector);
        newDir->Add("..", pSector);
        newDir->WriteBack(newDirFile);
        FileHeader *pHdr = new FileHeader();
        pHdr->FetchFrom(pSector);
        pHdr->setdirnum(directory->getSize());
        pHdr->WriteBack(pSector);
        delete pHdr;
        *name = newDirFile->getFullName();
        delete newDir;
        delete newDirFile;
      }
      delete dirHdr;
    }
    delete freeMap;
  }

  delete directory;
  delete openFile;
  return success;
  // add new dir to inode
}

int 
FileSystem::parsePath(char **name, int sector) { // find the corresponding file, [out] name will hold the full name
  // treat as relative path
  sector = processPath(name, sector);
  if (sector == -1) {
    // treat as full path
    sector = processPath(name, DirectorySector);
  }
  return sector; // sector of dir
}

int FileSystem::processPath(char **name, int sector) {
  string path = string(*name), dir;
  size_t pos;
  char *filename;
  //cout << *name << "]name\n";
 // cout << path << "]path\n"; int count = 0;
  // if is relative path
  while ((pos = path.find("/")) != string::npos) {
    dir = path.substr(0, pos);
    path = path.substr(pos + 1, path.size());

   // cout << dir << "] dir - " << count++  <<" \n";
    //cout << path << "] path - " << count++ << " \n";

    if (dir == "") {
      dir = ".";
    }

    Directory *dirObj = new Directory(NumDirEntries);
    OpenFile *dirFile = new OpenFile(sector);
    dirObj->FetchFrom(dirFile);
    if (dirObj->Find((char*)dir.c_str()) != -1) { // can find the dir
      sector = dirObj->Find((char*)dir.c_str());  // get sector of the dir
      delete dirObj;
      delete dirFile;
    }
    else { // cannot find
      return -1;
    }
  }

  filename = new char[path.length() + 1];
  strcpy(filename, path.c_str());
  *name = filename; // get file name 
  return sector; // sector of dir
}
 
void FileSystem::setCurrentDirSector(int sector) { // update by calling process
  currentDirSector = sector;
}

int 
FileSystem::getCurrentDirSector() {
  return currentDirSector;
}

char* 
FileSystem::getFullName(char *name, int sec) { // will change "name"
  int sector = parsePath(&name, sec);
  if (sector == -1) { // cannot open
    cout << "Error path! \n";
    return NULL;
  }

  OpenFile* openFile = new OpenFile(sector);
  Directory* directory = new Directory(NumDirEntries);
  directory->FetchFrom(openFile);
  sector = directory->Find(name);
  if (sector == -1) {
    cout << "Error filename! \n";
    return NULL;
  }

  char *result = getFullName(sector); // dir path
  delete openFile;
  delete directory;
  return result;
}

int 
FileSystem::getProtectionBit(char *name, int sec) { // will not change name
  FileHeader *hdr = new FileHeader();
  Directory *dir = new Directory(NumDirEntries);
  OpenFile *parentFile;

  char* temp = new char[strlen(name) + 1];
  memcpy(temp, name, strlen(name) + 1);

  int sector = parsePath(&temp, sec);
  int targetSector;
  if (sector == -1) { // treat as root
    sector = DirectorySector;
  }

  parentFile = new OpenFile(sector);
  dir->FetchFrom(parentFile);
  targetSector = dir->Find(temp);

  if (targetSector == -1) {
    targetSector = DirectorySector; // treat as root
  }
  delete parentFile;

  hdr->FetchFrom(targetSector);
  int result = hdr->getProtectionBit(); // dir path
  delete hdr;
  delete dir;
  return result;
}

bool 
FileSystem::canWrite(int bit) { // - w - (2), r w - (6), r w x (7), - w x (3)
  if (bit == 2 || bit == 6 || bit == 7 || bit == 3) {
    return TRUE;
  }
  return FALSE;
}

bool 
FileSystem::canRead(int bit) { // r - - (4), r w - (6), r - x (5), r w x (7)
  if (bit == 4|| bit == 6 || bit == 5 || bit == 7) {
    return TRUE;
  }
  return FALSE;
}

bool 
FileSystem::canExecute(int bit) { // - - x (1), r - x (5), - w x (3), r w x(7)
  if (bit == 1 || bit == 5 || bit == 3 || bit == 7) {
    return TRUE;
  }
  return FALSE;
}

char* 
FileSystem::getParentDirectory(char* fullPath){
  string path = string(fullPath), dir;
  size_t pos;
  char *result;

  pos = path.find_last_of("/");
  if (pos  != string::npos) {
    dir = path.substr(0, pos);
  }

  result = new char[dir.length() + 1];
  strcpy(result, dir.c_str());
  return result;
}

bool 
FileSystem::setProtectionBit(char* fullPath, int bit) {
  char* temp = new char[strlen(fullPath) + 1];
  memcpy(temp, fullPath, strlen(fullPath) + 1);

  if (!(bit <= 7 && bit >= 1)) {
    cout << "No Such protection bit!\n";
      return FALSE;
  }

  int sector = parsePath(&temp, currentDirSector);
  if (sector == -1) {
    cout << "No such path\n";
    return FALSE;
  }
  FileHeader *hdr = new FileHeader();
  OpenFile *openfile = new OpenFile(sector);
  Directory *directory = new Directory(NumDirEntries);
  directory->FetchFrom(openfile);
  int targetSector = directory->Find(temp); 

  FileHeader *parentHdr = new FileHeader();
  parentHdr->FetchFrom(sector);
  if (canWrite(parentHdr->getProtectionBit()) == FALSE){
    cout << "Privilege Denied: dir - " << parentHdr->getProtectionBit() << "\n";
    return FALSE;
  }

  hdr->FetchFrom(targetSector);
  hdr->setProtectionBit(bit); // dir path
  hdr->WriteBack(targetSector);
  delete openfile;
  delete directory;
  delete parentHdr;
  delete hdr;
  return 1;
}

char* 
FileSystem::getFullName(int sector) {
  string path = "";
  string name;
  char *filename = new char[100];

  if (sector < 1 || sector > 1029) {
    return "invalid";
  }

  while (sector != 1029) {
    FileHeader *currentHdr = new FileHeader();
    currentHdr->FetchFrom(sector);
    int parentSector = currentHdr->getParentSector();

    if (parentSector != 1029) { // not root
      OpenFile *parentFile = new OpenFile(parentSector);
      Directory* directory = new Directory(NumDirEntries);
      directory->FetchFrom(parentFile);
      char* temp  = directory->getName(sector);
      name = string(temp);
      if (path != "") {
        path = name + "/" + path;
      }
      else {
        path = name;
      }

      delete parentFile;
      delete directory;
    }
    else {
      // root dir
      ASSERT(sector == 1);
      path = "/" + path;
    }
    sector = parentSector;
    delete currentHdr;
  }

  strcpy(filename, path.c_str());
  return filename; // sector of dir
}

bool 
FileSystem::isEmptyDir(char* path, int sector) { // whether a path is dir or not
  char* temp = new char[strlen(path) + 1];
  memcpy(temp, path, strlen(path) + 1);
  int dirSector = parsePath(&temp, sector);
  if (dirSector == -1) {
    cout << "No such path\n";
    return FALSE;
  }
  FileHeader *hdr = new FileHeader();
  OpenFile *openfile = new OpenFile(dirSector);
  Directory *directory = new Directory(NumDirEntries); // parent
  directory->FetchFrom(openfile);
  int targetSector = directory->Find(temp);
  Directory *targetDir = new Directory(NumDirEntries);
  OpenFile *targetFile = new OpenFile(targetSector);
  targetDir->FetchFrom(targetFile);
  if (targetDir->isEmpty() == FALSE) {
    cout << "Fail, not an empty directory\n";
    return FALSE;
  }

  hdr->FetchFrom(targetSector);
  bool isDir = hdr->isDir();
  if (isDir == FALSE) {
    cout << "Fail, not a dir.\n";
  }

  delete hdr;
  delete openfile;
  delete directory;
  return isDir;
}

int 
FileSystem::moveDir(char* from, char *to) {
  // acquire sector
  // fetch dir
  // write to target dir
  // if success, delete current dir
  char* fromDir = new char[strlen(from) + 1];
  memcpy(fromDir, from, strlen(from) + 1);

  char* toDir = new char[strlen(to) + 1];
  memcpy(toDir, to, strlen(to) + 1);

  int fromPSector = parsePath(&fromDir, currentDirSector);
  int toPSector = parsePath(&toDir, currentDirSector);

  if (fromPSector == -1 || toPSector == -1) {
    cout << "No such path\n";
    return FALSE;
  }


  FileHeader *hdr = new FileHeader();
  OpenFile *fromOpenfile = new OpenFile(fromPSector);
  Directory *fromDirectory = new Directory(NumDirEntries);
  OpenFile *toOpenfile = new OpenFile(toPSector);
  Directory *toDirectory = new Directory(NumDirEntries);
  fromDirectory->FetchFrom(fromOpenfile);
  toDirectory->FetchFrom(toOpenfile);
  int targetSector = fromDirectory->Find(fromDir);
  if (targetSector == -1) {
    cout << "No such target file!\n";
    return FALSE;
  }

  hdr->FetchFrom(targetSector);

  // start check privilege //
  FileHeader *fromPHdr = new FileHeader();
  FileHeader *toPHdr = new FileHeader();
  fromPHdr->FetchFrom(fromPSector);
  toPHdr->FetchFrom(toPSector);
  if (canRead(fromPHdr->getProtectionBit()) == FALSE || canWrite(fromPHdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << from << " dir - " << fromPHdr->getProtectionBit() << "\n"; // should can read and write from parent dir
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }
  if (canWrite(toPHdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << to << " dir - " << toPHdr->getProtectionBit() << "\n";// should can write from parent dir
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }
  if (canRead(hdr->getProtectionBit()) == FALSE || canWrite(hdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << to << " dir - " << hdr->getProtectionBit() << "\n"; // should can read and write source file
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }

  delete fromPHdr;
  delete toPHdr;
  // end check privilege //

  hdr->setParentSector(toPSector); // update info
  hdr->WriteBack(targetSector);

  fromDirectory->Remove(fromDir); // change parent dir entry
  toDirectory->Add(toDir, targetSector);
  fromDirectory->WriteBack(fromOpenfile); // update
  toDirectory->WriteBack(toOpenfile);

  if (hdr->isDir() == TRUE) { // if the path is dir path
    OpenFile *openfile = new OpenFile(targetSector);
    Directory *dirFile = new Directory(NumDirEntries);
    dirFile->FetchFrom(openfile);

    if (dirFile->Find("..") != -1) { // update entry
      dirFile->Remove(".."); 
      dirFile->Add("..", toPSector);
      dirFile->WriteBack(openfile); // save
    }

    delete openfile;
    delete dirFile;
  }

  delete hdr;
  delete fromOpenfile;
  delete toOpenfile;
  delete fromDirectory;
  delete toDirectory;
  return TRUE;
}

int 
FileSystem::copyFile(char* from, char *to) {

  int amountRead, fileLength;
  char *buffer;

  char* fromDir = new char[strlen(from) + 1];
  memcpy(fromDir, from, strlen(from) + 1);

  char* toDir = new char[strlen(to) + 1];
  memcpy(toDir, to, strlen(to) + 1);

  DEBUG(dbgFile, "From:" << from);
  DEBUG(dbgFile, "To:" << to);

  int fromPSector = parsePath(&fromDir, currentDirSector);
  int toPSector = parsePath(&toDir, currentDirSector);

  DEBUG(dbgFile, "From:" << fromPSector);
  DEBUG(dbgFile, "To:" << toPSector);

  if (fromPSector == -1 || toPSector == -1) {
    cout << "No such path\n";
    return FALSE;
  }

  FileHeader *hdr = new FileHeader();
  OpenFile *fromOpenfile = new OpenFile(fromPSector);
  Directory *fromDirectory = new Directory(NumDirEntries);
  OpenFile *toOpenfile = new OpenFile(toPSector);
  Directory *toDirectory = new Directory(NumDirEntries);
  fromDirectory->FetchFrom(fromOpenfile);
  int targetSector = fromDirectory->Find(fromDir);
  if (targetSector == -1) {
    cout << "No such target file!\n";
    return FALSE;
  }

  hdr->FetchFrom(targetSector);

  // start check privilege //
  FileHeader *fromPHdr = new FileHeader();
  FileHeader *toPHdr = new FileHeader();
  fromPHdr->FetchFrom(fromPSector);
  toPHdr->FetchFrom(toPSector);
  if (canRead(fromPHdr->getProtectionBit()) == FALSE || canWrite(fromPHdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << from << " dir - " << fromPHdr->getProtectionBit() << "\n"; // should can read and write from parent dir
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }
  if (canWrite(toPHdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << to << " dir - " << toPHdr->getProtectionBit() << "\n";// should can write from parent dir
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }
  if (canRead(hdr->getProtectionBit()) == FALSE || canWrite(hdr->getProtectionBit()) == FALSE) {
    cout << "Privilege Denied: file - " << to << " dir - " << hdr->getProtectionBit() << "\n"; // should can read and write source file
    delete fromPHdr;
    delete toPHdr;
    return FALSE;
  }

  delete fromPHdr;
  delete toPHdr;
  // end check privilege //

  //read data
  OpenFile *fromFile = new OpenFile(targetSector);

  // Create a Nachos file of the same length
  fileLength = hdr->FileLength();
  DEBUG('f', "Copying file " << from << " of size " << fileLength << " to file " << to);
  if (!kernel->fileSystem->Create(to, fileLength, toPSector, 7)) {   // Create Nachos file
    printf("Copy: couldn't create output file %s\n", to);
    return FALSE;
  }

  OpenFile *openFile;
  openFile = kernel->fileSystem->Open(to);
  ASSERT(openFile != NULL);

  buffer = new char[fileLength + 1];
  fromFile->Read(buffer, fileLength);
  openFile->Write(buffer, fileLength);
  delete buffer;

  // Close the Nachos files
  delete openFile;
  delete fromFile;
  delete hdr;
  delete fromOpenfile;
  delete fromDirectory;
  delete toOpenfile;
  delete toDirectory;

  cout << "The -cp operation is completed.\n";
}

#endif // FILESYS_STUB
