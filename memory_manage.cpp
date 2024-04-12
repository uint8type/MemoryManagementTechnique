#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <cstdlib>
#include <set>
#include <ctime>
#include <cstring>
// #include<>
#define blocks 16384
#define INT_MIN -99999
using namespace std::chrono;
#define maxDataSize 10
struct Page
{
    int frameNo;   // page id
    bool isLoaded; // is the page loaded in memory?
    bool isDirty;  // has the page been modified since it was loaded?
    char *data;    // pointer to the page data
    int size;
    std::chrono::high_resolution_clock::time_point firstEntry;   // field that holds the time when it is first entered
    std::chrono::high_resolution_clock::time_point lastAccessed; // time when the page is last accessed
};

struct ramEntry
{
    // int frameNo;
    int pageNo;
    bool empty; // whether that entry holding any page currently
};
// it is block in disk
struct hardDiskEntry
{
    char *str;  // holds the data
    int pageno; // which page is mapped to this block
    int size;   // size of the string that it holds
};
class MemoryManager // memory manager class
{
public:
    MemoryManager(int pageSize, int numPages, std::string algorithm, int memSize, int swapspace)
    {
        // initializing all attributes
        numFrames = memSize / pageSize;
        this->pageSize = pageSize;
        this->numPages = numPages;
        swapSize = pageSize * numPages;

        this->algorithm = algorithm;
        numPageAccess = 0;
        numPageFaults = 0;
        ram.resize(numFrames);
        pageTable.resize(numPages);
        //   pagetoDisk.resize(numPages);
        memory.resize(memSize);
        hardDisk.resize(blocks);
        // std::fstream swapSpace("swap_space.bin", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

        // swapSpace.seekp((numPages * pageSize) - 1, std::ios::beg);

        // swapSpace.write("", 1);
        // initialising page table
        for (int i = 0; i < numPages; i++)
        {
            pageTable[i].frameNo = -1;
            pageTable[i].isDirty = false;
            pageTable[i].isLoaded = false;
            // pageTable[i].
        }
        // initializing ram
        for (int i = 0; i < numFrames; i++)
        {
            ram[i].empty = true;
            ram[i].pageNo = -1;
            // pageTable[i].
        }
        // initializing blocks of disk
        for (int i = 0; i < numPages; i++)
        {
            hardDisk[i].pageno = i;
            pagetoDisk[i] = i;
            hardDisk[i].size = 0;
        }
    }

    // ~MemoryManager();
    // methods of the class
    void loadPage(int pageNo);               // function to load and read a page
    void unloadPage(int pageNo);             // function to unload any particular page
    void writePage(int pageNo, char *data);  // given page number and data, the data is written on given page number
    void writeinram(int x, int y, char *yj); // this function writes in ram
    int getPageFaults();                     // getter
    int getPageAccess();                     // getter

private:
    std::vector<Page> pageTable; // vector of pages
                                 // std::unordered_map<int, Page> pageTable;  // map of page no to frame no in the vector
    std::vector<ramEntry> ram;
    std::unordered_map<int, int> pagetoDisk;
    //    std::vector<ramEntry> ram;
    std::vector<char> memory;
    int pageSize; // size of each page
    int numPages; // total number of pages in memory
    int swapSize; // size of the swap space
    int numFrames;
    std::string algorithm; // page replacement algorithm
    // std::fstream swapSpace;  // pointer to the swap space
    int replacePage(int id); // private function to replace a page using the specified algorithm
    void print(int pageNo);  // print content saved in a page
    int numPageFaults;
    int numPageAccess;
    std::vector<hardDiskEntry> hardDisk;
};
int MemoryManager::getPageAccess() // getter function
{
    return numPageAccess;
}

int MemoryManager::getPageFaults() // getter function
{
    return numPageFaults;
}
void MemoryManager::print(int id) // prints content of a page
{
    int frame = pageTable[id].frameNo;

    for (int i = frame * pageSize; i < frame * pageSize + pageTable[id].size; i++)
    {
        std::cout << memory[i]; // data saved on memeory
    }
    std::cout << "\n"; // line change
}
// writting constructors

// takes a page number if that is already loaded just update last acsessed and return
// finds in ram if there is any empty field
//  if yes set it target frameNumber
// if no one is empty then call the replacement algorithm that returns target frame number
// traverse the pagetable the page was previosuly saved in that framenumber
// make that entry isloaded =false; and that dirty bit is checkefd
// update fields of current pageId
void MemoryManager::loadPage(int pageNo)
{
    numPageAccess++;
    if (pageTable[pageNo].isLoaded) // if it is already loaded in that case it is hit
    {
        // Page hit
        pageTable[pageNo].lastAccessed = std::chrono::high_resolution_clock::now();
        print(pageNo); // read the content of the page
        return;        // page is already in memory returnn
    }
    // Page fault
    pageTable[pageNo].firstEntry = std::chrono::high_resolution_clock::now(); // fault so update first entry
    numPageFaults++;                                                          // page isn't loaded
    int frameNumber = -1;                                                     // framenumber where page tries to sit
    for (int i = 0; i < ram.size(); i++)
    {
        if (ram[i].empty == true) // if that ram entry is empty
        {
            frameNumber = i; // got the targeted frame number
            break;
        }
    }
    if (frameNumber == -1) // memory full page replacement algo
    {

        frameNumber = replacePage(pageNo);
        // frameNo = map[id];
    }
    ram[frameNumber].empty = false;

    // iterate the page table
    for (int i = 0; i < pageTable.size(); i++)
    {
        if (pageTable[i].frameNo == frameNumber) // got the page who is mapped to target frame
        {
            pageTable[i].isLoaded = false; // make corresponding page unloaded

            if (pageTable[i].isDirty == true) // it needs write back in disk
            {
                // write this page in swap space
                int disknum = pagetoDisk[i];
                hardDisk[disknum].str = pageTable[i].data;              // write data in block
                hardDisk[pagetoDisk[disknum]].size = pageTable[i].size; // update the data's size
                pageTable[i].isDirty = false;                           // mark isDirty=false
            }
            break;
        }
    }
    // update fields in the new entry
    pageTable[pageNo].frameNo = frameNumber;
    pageTable[pageNo].isLoaded = true;
    pageTable[pageNo].isDirty = true;
    pageTable[pageNo].lastAccessed = std::chrono::high_resolution_clock::now();
    pageTable[pageNo].firstEntry = std::chrono::high_resolution_clock::now();

    // swapSpace.seekg(pageNo * pageSize, std::ios::beg);
    // swapSpace.read(reinterpret_cast<char *>(&memory[frameNumber * pageSize]), pageSize);

    // write in ram
    writeinram(frameNumber, hardDisk[pagetoDisk[pageNo]].size, hardDisk[pagetoDisk[pageNo]].str);

    print(pageNo);
}
// this function writes data into ram
void MemoryManager::writeinram(int framenumber, int size, char *data)
{
    int offset = framenumber * pageSize; // from where writting will start
    int j = 0;

    for (int i = offset; i < offset + size; i++)
    {
        memory[i] = data[j++]; // data is copied in ram
        // std::cout<<memory[i];
    }
    for (int i = 0; i < size; i++)
    {
        // std::cout << data[i];
    }
}
// unload is a function that takes page id
// if that is loaded in ram sets that corresponding field is set to empty
// pagetable is updayted for the page
// transfered to swap space if isDirty==true
void MemoryManager::unloadPage(int id)
{
    if (pageTable[id].isLoaded == true) // it means it is in ram
    {

        for (int i = 0; i < ram.size(); i++)
        {
            if (ram[i].pageNo == id) // get the page number
            {
                ram[i].empty = true; // make the frame empty
                break;
            }
        }
        pageTable[id].isLoaded = false; // make it not loaded
        //  pageTable[id].

        if (pageTable[id].isDirty == true)
        {
            // disk transfer

            hardDisk[pagetoDisk[id]].size = pageTable[id].size;
            hardDisk[pagetoDisk[id]].str = pageTable[id].data;
        }
    }
}
// given any data and page id it writes data in that page
void MemoryManager::writePage(int id, char *data)
{

    if (pageTable[id].isLoaded == true) // if loaded in ram
    {

        // if(id==1)
        // {
        //     for(int i=0;i<strlen(data);i++)
        //     {
        //         std:: cout<<data[i];
        //     }
        // }
        numPageAccess++;
        pageTable[id].isDirty = true;      // mark dirty bit true
        pageTable[id].size = strlen(data); // update size
        pageTable[id].data = data;         // copy data
        // declare a physical   mem and refer syntax from don's code
        // memcpy(reinterpret_cast<char *>(&memory[pageTable[id].frameNo * pageSize]), data, pageSize);
        writeinram(pageTable[id].frameNo, strlen(data), data);

        // if (id == 1)
        // std::cout << pageTable[id].frameNo<< "\n";

        print(id); // prints the content of the page
        return;
    }
    else
    {
        // page fault
        numPageFaults++;
        pageTable[id].firstEntry = std::chrono::high_resolution_clock::now(); // update first entru
        int frameNumber = -1;                                                 // initialize target frame number
        for (int i = 0; i < ram.size(); i++)                                  // traverse the ram
        {
            if (ram[i].empty == true) // if an entry is empty
            {
                frameNumber = i; // update the target frame number
                break;
            }
        }
        // std::cout << "ak\n";
        if (frameNumber == -1) // if no entry in ram is empty
        {

            // memory full page replacement algo
            frameNumber = replacePage(id);
        }
        // update necessary fields
        ram[frameNumber].empty = false;
        pageTable[id].isLoaded = true;
        pageTable[id].frameNo = frameNumber;
        ram[frameNumber].pageNo = id;

        writePage(id, data); // write the data in the page
    }
}
// LRU && FIFO both implememted
int MemoryManager::replacePage(int id)
{
    int index = -1;
    int target = -1;
    //    std:: cout<<"ak\n";
    int max = INT_MIN;
    if (algorithm == "LRU") // LRU
    {
        // std:: cout<<"ak\n";
        std::chrono::high_resolution_clock::time_point min = pageTable[0].lastAccessed;
        //   std:: cout<<"ak\n";
        for (int i = 0; i < ram.size(); i++)
        {
            // std:: cout<<"ak\n";
            int currentPageNumber = ram[i].pageNo;
            // std::cout << "ak\n";
            // std:: cout<<currentPageNumber;
            if (pageTable[currentPageNumber].lastAccessed < min)
            {
                min = pageTable[currentPageNumber].lastAccessed; // find the page who has least last access
                target = i;
            }
        }
    }
    else
    {
        // FIFO

        std::chrono::high_resolution_clock::time_point max = pageTable[ram[0].pageNo].firstEntry;
        for (int i = 1; i < ram.size(); i++)
        {
            int currentPageNumber = ram[i].pageNo; // get the current pagenumber
            if (pageTable[currentPageNumber].firstEntry < max)
            {
                max = pageTable[currentPageNumber].lastAccessed;
                target = i; // update the target frame
            }
        }
    }
    // std:: cout<<"ak\n";
    unloadPage(ram[target].pageNo);
    return target;
}

///////////////

class DataGenerator // this class generates data
{
public:
    DataGenerator(int numOfPages, int pageSize)
    {
        // initializations in constructor
        this->numOfPages = numOfPages;
        this->pageSize = pageSize;
    }
    // functions or methods
    int generatePageId();
    char *generateRandomDat();
    // ~MemoryManager();
    // void loadPage(int pageNo);

private:
    int numOfPages; // attributes
    int pageSize;   // attributes
};

int DataGenerator::generatePageId() // this function randomly generates page id
{
    // srand(time(0));
    int x = rand() % numOfPages; // randomly generates page id
    return x;
}
char *DataGenerator::generateRandomDat()
{
    // make random data which fits into a frame..
    // srand(time(0));
    // int size = 1 + rand() % pageSize;
    int size = 1 + rand() % maxDataSize;
    // genearetes random data of any size upto max data size
    char *gendata = new char[size + 1];
    for (int i = 0; i < size; i++)
    {
        //  srand(time(0));
        gendata[i] = (char)(33 + rand() % 94); // generates data and save in gendata
    }
    gendata[size] = '\0'; // null termination
    return gendata;       // return generated data
}
void generatereference(int count, std::string algo, int pageSiz, int numPage, int swapspac, int memSiz)
{
    // this function triggers paging paradigm
    //  srand(time(0));
    // values are initialized
    int pageSize = pageSiz;
    int numPages = numPage;
    int swapspace = swapspac;

    std::string algorithm = algo;
    int memSize = memSiz;
    std::set<int> mySet;                                                     // set that keeps track of already written pages
    DataGenerator dataGen(numPages, pageSize);                               // calls data generator
    MemoryManager paging(pageSize, numPages, algorithm, memSize, swapspace); // creats object of paging class
    // declare a set of pages who are written lets say that is written pages

    for (int i = 0; i < count; i++) // number of times data is generated
    {
        int x = rand() % 2;             // read or write
        int pageNo = rand() % numPages; // randomly generates page number
        if (x == 0)
        { // reading

            // if generated page id already isn't written
            if (mySet.find(pageNo) == mySet.end())
            {
                std::cout << "requested page id isn't written before.. \n";
                std::cout << "we are writting randomly generated data.... \n";
                char *randdata = dataGen.generateRandomDat(); // generates random data
                paging.writePage(pageNo, randdata);           /// writes on randomly generated page number
                std::cout << "writting over..\n"
                          << "written data is ..\n";

                // you can read the page also//better way to solve
                for (int i = 0; i < strlen(randdata); i++)
                {
                    std::cout << randdata[i]; // print the written data
                }
                std::cout << "\n";
            }
            else
            {
                // generated pageid is written before

                paging.loadPage(pageNo);
            }
        }
        else
        { // writting
            // add the page number into the set
            mySet.insert(pageNo);
            std::cout << "we are writting randomly generated data.... \n";
            char *randdata = dataGen.generateRandomDat();
            paging.writePage(pageNo, randdata);
            std::cout << "writting over..\n written data is ..\n";

            // read from load function
            for (int i = 0; i < strlen(randdata); i++)
            {
                std::cout << randdata[i];
            }
            std::cout << "\n";
        }
    }

    std::cout << " Number of page fauts" << paging.getPageFaults() << "\n";
    std::cout << " Number of page accesses" << paging.getPageAccess() << "\n";

    double missratio = (double)paging.getPageFaults() / (double)paging.getPageAccess();

    std::cout << missratio * 100 << "%"
              << "\n";
}

/// Below is the nonPaging implementation
class NonPaging
{
public:
    NonPaging(int memSize, int numProcess) // constructor
    {
        // initialization
        lastProcessid = -1;
        numRejectedProcess = 0;
        this->memSize = memSize;
        this->numProcess = numProcess;
        freeSpaceTracker.resize(memSize);
        for (int i = 0; i < memSize; i++)
        {
            freeSpaceTracker[i] = false; // initialize free space tarcker
        }
    }
    // function that allocate a process
    int allocate(int processNo);
    // deallocate a process
    int deallocate(int processNo);

    // randomly generates rocess
    void processGenerator();
    // getter
    int getnumRejectedProcess()
    {
        return numRejectedProcess;
    }

private:
    //  std:: vector<int> ram;
    int numProcess;
    int memSize;
    int lastProcessid;
    int numRejectedProcess;
    std::vector<bool> freeSpaceTracker;             // tracks free space in memory
    std::unordered_map<int, int> pageToSize;        // stores size of each process
    std::unordered_map<int, int> pageToMappedIndex; // process started mapping from which index
};
void NonPaging::processGenerator()
{

    for (int i = 0; i < numProcess; i++) // how many processes you want to generate
    {
        int size = rand() % memSize;
        pageToSize[++lastProcessid] = size; // store the size of the processor
        std::cout << "generated process id is " << lastProcessid;

        std::cout << "and size is " << size << "\n";
        int allocationPossible = allocate(lastProcessid); // it return 1 if allocation possible else returns -1
                                                          // and allocates the process
        if (allocationPossible == 1)
        {
            // allocation done
            // allocation done
            std::cout << "yeah!! Allocated \n";
        }
        else
        {
            // rejected process
            numRejectedProcess++;

            pageToSize.erase(lastProcessid); // erase from database
            std::cout << "Oops !! Sorry couldn't be alloctaed \n";
        }
    }
}
int NonPaging::allocate(int processNo) // function that allocates a process
{

    int size = pageToSize[processNo]; // get size of the process

    int index = -1;
    // std:: cout<<size<<"\n";
    // find contiguous run of false that fits the process size
    for (int i = 0; i < freeSpaceTracker.size();)
    {
        int j = i;
        int flag = 1;
        if (freeSpaceTracker[i] == false)
        {

            int count = 0;
            j = i;
            // if current val is false that says that is empty cell
            while (j < memSize && freeSpaceTracker[j] == false)
            {
                count++;           // increment counbt
                if (count >= size) // if current count is >= size
                {
                    index = i; // update index
                    break;
                }
                j++; // update j
            }

            if (index == -1)
            {
                // flag=0;
                i = j;
            }
            else
            {
                flag = 0;
            }
        }
        if (flag == 1)
            i++;
        else
            break;
    }

    // std::cout<<index<<"\n";

    if (index == -1)
    {
        // no room for allocation
        int deallocationPossible = deallocate(processNo);
        if (deallocationPossible == 1)
        {
            allocate(processNo);
        }
        else
        {
            // even after removing largest process room can't be found for the current process
            return -1;
        }
    }
    else
    {
        // room is found
        for (int bi = index; bi < pageToSize[processNo] + index; bi++)
        {
            freeSpaceTracker[bi] = true; // mark them as true
        }
        pageToMappedIndex[processNo] = index; // update the index
        return 1;
    }
}
int NonPaging::deallocate(int processNo)
{
    int size = pageToSize[processNo];          // update size
    std::unordered_map<int, int>::iterator it; // initialize iterator
    int max = INT_MIN;
    int maxprocessid = -1;
    for (it = pageToMappedIndex.begin(); it != pageToMappedIndex.end(); ++it)
    {
        // std::cout << it->first << ": " << it->second << std::endl;

        int process = it->first;
        // find the process with max process size
        if (pageToSize[process] >= max)
        {
            max = pageToSize[process];
            maxprocessid = process;
        }
    }
    // segmentation -1
    int maxindex = pageToMappedIndex[maxprocessid];
    int left = 0;
    int in = maxindex - 1;
    // find free space in left
    while (in >= 0 && freeSpaceTracker[in] == false)
    {
        left++;
        in--;
    }
    in = maxindex + pageToSize[maxprocessid];
    int right = 0;
    // find free space in right
    while (in < memSize && freeSpaceTracker[in] == false)
    {
        right++;
        in++;
    }
    // total size>=size or not
    if (max + left + right >= size)
    {
        // can be allocated
        // deallocate
        // from where proces startefd
        int start = pageToMappedIndex[maxprocessid];
        int len = pageToSize[maxprocessid]; // size of the process

        for (int bit = start; bit < start + len; bit++)
        {
            freeSpaceTracker[bit] = false; // update space tracker
        }
        return 1;
    }
    else
    {
        // can't be allocated even aftyer deallocating largest process
        return -1;
    }
}
int main()
{
    high_resolution_clock::time_point t1Paging = high_resolution_clock::now();
    srand(time(0));

    // under function works for paging
    int pagingreference = 100000;
    // std:: string= "LRU";
    int pageSize = 1024;
    int numPages = 1024;
    int swapspace = 1024 * 1024;

    int memSize = 1024 * 8;

    // this function triggers paging system
    generatereference(pagingreference, "LRU", pageSize, numPages, swapspace, memSize);

    high_resolution_clock::time_point t2Paging = high_resolution_clock::now();
    // time used to get the time needed in paging

    long long durationPaging = duration_cast<seconds>(t2Paging - t1Paging).count();
    // std::cout << "Time needed in paging " << durationPaging;
    int numofAccessInContiguous = 10000;

    // nonpaging object
    int nonPagingmemSize = 1024 * 16;
    NonPaging nonPaging(nonPagingmemSize, numofAccessInContiguous);
    high_resolution_clock::time_point t1nonPaging = high_resolution_clock::now();
    // this generates random size process and simulates
    nonPaging.processGenerator();
    // prints number of rejected processes
    std::cout << nonPaging.getnumRejectedProcess() << "\n";
    // rejected process percentage calculation
    double rejectionRatio = (double)nonPaging.getnumRejectedProcess() / (double)numofAccessInContiguous;
    std::cout << "Process rejection  ratio " << rejectionRatio * 100 << " % \n";
    high_resolution_clock::time_point t2nonPaging = high_resolution_clock::now(); // time stamp to calculate time needed

    long long durationnonPaging = duration_cast<seconds>(t2nonPaging - t1nonPaging).count();
    std::cout << "Time needed in paging " << durationPaging << "\n";
    std::cout << "Time needed in nonpaging " << durationnonPaging << "\n";
}