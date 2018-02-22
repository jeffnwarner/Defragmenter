#include <iostream>
#include <stdlib.h>  // for atoi
#include "DefragRunner.h"
#include "defragmenter.h"
#include "QuadraticProbing.h"
#include "mynew.h"
#include "QueueAr.h"

using namespace std;

Defragmenter::Defragmenter(DiskDrive *diskDrive)
{
	//A Disk Drive has a capacity, disk, directory, FAT, and disk accesses.
	//A Directory has a FileID firstBlock ID and a size.
	//A Disk Block has FileID FileBlockNum and NextBlockID.
  DiskBlock_Number dbn(0,0);
	int currentBlock = 2; //Start diskBlocks from 2.
	int startingBlock = 2;
	int tempNum = 0;
  QuadraticHashTable <DiskBlock_Number> hash_table(dbn, 200000); //creation of hash table

  Queue <int> queue (50000);

  for(int i = 0; i < 10000; i++)
  {
    if(i < diskDrive->getCapacity() && !diskDrive->FAT[diskDrive->getCapacity() - i] && !queue.isFull())
    { 
      queue.enqueue(diskDrive->getCapacity() - i);
      tempNum++;
    }
  } //fill queue with empty spots
  
	for (int i = 0; i < diskDrive->getNumFiles(); i++) //for how many files are in the directory
	{
	  int blockID = diskDrive->directory[i].getFirstBlockID();//get first block ID of the directory.
      
	  while(blockID > 1)//If blockID is 1 or 0 then stop because nothing should be in there.
	  {
        DiskBlock_Number temp;
        DiskBlock_Number temp2; 
        DiskBlock_Number dummy(blockID, 0);
        while((temp = hash_table.find(dummy)).original != 0)
        {
          //location = temp.current;//returns current location of the diskBlock desired.
          //temp = hash_table.find(location);//continues loop just in case.
          hash_table.remove(dummy);
          temp2 = temp;
          dummy.original = temp.current;
         
        }//while 
        //}//if statement
        //else
          //location = -1;//nextDiskBLock will never be -1
        if (temp2.current > 0)
          blockID = temp2.current;

        DiskBlock *diskBlock = diskDrive->readDiskBlock(blockID);//temporary diskBlock

        if (!diskDrive->FAT[currentBlock])//If the spot is unused
        {
          if (!queue.isFull())
            queue.enqueue(blockID);
          arrange(diskDrive, currentBlock, blockID, diskBlock);//speaks for itself right?
        }
        else
        {
          //check hash table to see if the spot has been moved
          int empty_spot = queue.getFront();//find the next empty spot.
          queue.dequeue();
          tempNum--;
          while(diskDrive->FAT[empty_spot] && !queue.isEmpty())
          {
            empty_spot = queue.getFront();
            queue.dequeue();
            tempNum--;
           // cout << "Current Size: " << tempNum << endl; 
          }
          moveBlockDown(diskDrive, currentBlock, empty_spot);
          DiskBlock_Number disk_block_num(currentBlock, empty_spot);//insert change of original number and changed number
          hash_table.insert(disk_block_num);
          //then
          if (!queue.isFull())
            queue.enqueue(blockID);
          arrange(diskDrive, currentBlock, blockID, diskBlock);
        }// else statement
        //int location = blockID;
        //DiskBlock_Number temp = hash_table.find(blockID);//finds the diskBlock in the hashTable
        //if (temp.original == blockID)//in case the hash Table returned null, checking if actually found
        //{
        
	  }//while loop
	  diskDrive->directory[i].setFirstBlockID(startingBlock);//So directory points to the correct diskBlock
	  startingBlock = currentBlock;//starts next directory at the correct diskBlock
    
	}//Operates under the assumption that the while loop is successful to some degree

	//Take disk drive and look at first file in the directory and take FirstBlockID to find the diskBlock in the disk. 
	//Move the diskBlock up to diskBlock 2 and if something is in 2 then move that down to the next unused block.
	//Look at the NextBlockId and find that block in the disk and move it to block 3 etc.
	//When NextBlockID is 0 or 1 you are done. Move onto the next file in the directory. 
} //Defragmenter constructor

void Defragmenter::arrange(DiskDrive *diskDrive, int& currentBlock, int& blockID, DiskBlock *diskBlock)
{
	diskDrive->FAT[currentBlock] = true;//diskBlock is now occupied.
  diskDrive->FAT[blockID] = false;//old spot is now unused
  //queue.enqueue(blockID);
  currentBlock++;//move down spot for later.
  blockID = diskBlock->getNext();//getNextBlockID to get the next block
  diskBlock->setNext(currentBlock);//SetNextBlock to the next immediate block.
  diskDrive->writeDiskBlock(diskBlock, currentBlock - 1);//write the diskBlock to the spot.
  delete diskBlock;
} //arrange()

/*int Defragmenter::findDiskBlock(QuadraticHashTable& hash_table, int& blockNum)
{
  int location;
  DiskBlock_Number *temp = hash_table.find(blockNum);//finds the diskBlock in the hashTable
  if (temp)//in case the hash Table returned null, checking if actually found
  {	
    while(temp)
    {
      location = temp.getCurrent();//returns current location of the diskBlock desired.
      temp = hash_table.find(blockNum);//continues loop just in case.
    }//while 
    return location;
  }
  else
  	return -1;//nextDiskBLock will never be -1
}//findDiskBlock
*/
void Defragmenter::moveBlockDown(DiskDrive *diskDrive, int& currentBlock, int& empty_spot)
{
	//while(diskDrive->FAT[empty_spot]) // while the spot is being used
   // empty_spot--; //keep incrementing till an empty spot is found
  DiskBlock *moveBlock = diskDrive->readDiskBlock(currentBlock);//need diskBlock for function.
  diskDrive->writeDiskBlock(moveBlock, empty_spot);
  diskDrive->FAT[currentBlock] = false;
  diskDrive->FAT[empty_spot] = true;
  delete moveBlock;
}