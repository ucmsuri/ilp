/* 
 * This file is part of MICA, a Pin tool to collect
 * microarchitecture-independent program characteristics using the Pin
 * instrumentation framework. 
 *
 * Please see the README.txt file distributed with the MICA release for more
 * information.
 */
#include "mica.h"
#include "mica_utils.h"
#include <iostream>
#include <algorithm>

#define outoforder
//#define myoutoforder
//#define inorder

void init_ilp_all();
void init_ilp_one();

VOID instrument_ilp_all(INS ins, ins_buffer_entry* e);
VOID instrument_ilp_one(INS ins, ins_buffer_entry* e);

VOID fini_ilp_all(INT32 code, VOID* v);
VOID fini_ilp_one(INT32 code, VOID* v);

/* support for fast instrumentation of all characteristics in a single run (avoid multiple InsertCalls!) */
void ilp_buffer_instruction_only(void* _e, THREADID tid);
VOID PIN_FAST_ANALYSIS_CALL ilp_buffer_instruction_only(void* _e, THREADID tid);
//void ilp_buffer_instruction_read(ADDRINT read1_addr, ADDRINT read_size);
VOID PIN_FAST_ANALYSIS_CALL ilp_buffer_instruction_read(ADDRINT read1_addr, ADDRINT read_size, THREADID tid);
//void ilp_buffer_instruction_read2(ADDRINT read2_addr);
VOID PIN_FAST_ANALYSIS_CALL ilp_buffer_instruction_read2(ADDRINT read2_addr, THREADID tid);
//void ilp_buffer_instruction_write(ADDRINT write_addr, ADDRINT write_size);
VOID PIN_FAST_ANALYSIS_CALL ilp_buffer_instruction_write(ADDRINT write_addr, ADDRINT write_size, THREADID tid);
ADDRINT ilp_buffer_instruction_next( THREADID tid);
/*ADDRINT ilp_buffer_instruction_2reads_write(void* _e, ADDRINT read1_addr, ADDRINT read2_addr, ADDRINT read_size, ADDRINT write_addr, ADDRINT write_size);
ADDRINT ilp_buffer_instruction_read_write(void* _e, ADDRINT read1_addr, ADDRINT read_size, ADDRINT write_addr, ADDRINT write_size);
ADDRINT ilp_buffer_instruction_2reads(void* _e, ADDRINT read1_addr, ADDRINT read2_addr, ADDRINT read_size);
ADDRINT ilp_buffer_instruction_read(void* _e, ADDRINT read1_addr, ADDRINT read_size);
ADDRINT ilp_buffer_instruction_write(void* _e, ADDRINT write_addr, ADDRINT write_size);
ADDRINT ilp_buffer_instruction(void* _e);*/
VOID empty_ilp_buffer_all();
extern UINT32 _ilp_win_size;
extern UINT32 _block_size; 

#define ILP_BUFFER_SIZE 1

#ifndef CLASS_ILP
#define CLASS_ILP

//ofstream is_file;
//is_file.open("is_file", ios::out|ios::trunc);

typedef struct ilp_buffer_entry_type{

        ins_buffer_entry* e;

        ADDRINT mem_read1_addr;
        ADDRINT mem_read2_addr;
        ADDRINT mem_read_size;

        ADDRINT mem_write_addr;
        ADDRINT mem_write_size;

} ilp_buffer_entry;


class ilp_buff
{
public:
UINT32 ilp_buffer_index;
ilp_buffer_entry* ilp_buffer[ILP_BUFFER_SIZE];
ilp_buff() 
{ 
//	cout << "creating buffer" << endl;
	int i;

        ilp_buffer_index = 0;
        for(i=0; i < ILP_BUFFER_SIZE; i++){
                if((ilp_buffer[i] = (ilp_buffer_entry*)malloc(sizeof(ilp_buffer_entry))) == (ilp_buffer_entry*)NULL){
                        cerr << "Could not allocate memory for ilp_buffer[" << i << "]" << endl;
                        exit(1);
                }
                ilp_buffer[i]->e = (ins_buffer_entry*)NULL;
                ilp_buffer[i]->mem_read1_addr = 0;
                ilp_buffer[i]->mem_read2_addr = 0;
                ilp_buffer[i]->mem_read_size = 0;
                ilp_buffer[i]->mem_write_addr = 0;
                ilp_buffer[i]->mem_write_size = 0;
					}
}
};

class my_ilp
{
public:
INT64 cpuClock_interval;
UINT64 timeAvailable[MAX_NUM_REGS];
nlist* memAddressesTable[MAX_MEM_TABLE_ENTRIES];
UINT32 windowHead;
UINT32 windowTail;
UINT64 cpuClock;
UINT64* executionProfile;
UINT64 issueTime;
UINT32 win_size;
UINT32 ilp_block_size;
INT32 size_pow_times;
INT64 index_all_times;
UINT64* all_times;
UINT32 win_ptr;
UINT32 issue_ptr;
ilp_buff buff;
my_ilp() { 
	//cout << "class created" << endl;
	UINT32 i;
	//win_size = _ilp_win_size;
	win_size = 8;
	ilp_block_size = 4; 
	cout << win_size << " " << ilp_block_size << endl;
	//ilp_block_size = _block_size; 
	size_pow_times = 10;
	if((all_times = (UINT64*)malloc((1 << size_pow_times) * sizeof(UINT64))) == (UINT64*)NULL){
		cerr << "Could not allocate memory" << endl;
		exit(1);
	}
	index_all_times = 1; // don't use first element of all_times
	windowHead = 0;
	windowTail = 0;
	cpuClock = 0;
	cpuClock_interval = 0;
	for(i = 0; i < MAX_NUM_REGS; i++){
		timeAvailable[i] = 0;
	}

	if((executionProfile = (UINT64*)malloc(win_size*sizeof(UINT64))) == (UINT64*)NULL){
		cerr << "Not enough memory (in main)" << endl;
		exit(1);
	}

	for(i = 0; i < win_size; i++){
		executionProfile[i] = 0;
	}
	issueTime = 0;
	issue_ptr = 0;
	win_ptr = 0;

}

void ilp_instr_full_one(){
//const UINT32 win_size_const = win_size;
	UINT32 win_size_const = win_size;
        /* set issue time for tail of instruction window */
       // if (windowTail == 0)
	#ifdef inorder
	UINT32 reordered;
        //if (windowTail == 0)
	executionProfile[windowTail] = issueTime;
        //else
	//executionProfile[windowTail] = max(executionProfile[windowTail-1],issueTime);
        //cout << windowTail << " " << win_size <<" " << win_size_const << endl;
	windowTail = (windowTail + 1) % win_size_const;
        cout << " : " << cpuClock << endl;

	/*cout << cpuClock << " : " ;
	for (int i=0;i<win_size;i++)
		cout << executionProfile[i] << ",";
	cout << endl;*/
        if(cpuClock < issueTime)
		{
			cpuClock = max(cpuClock,issueTime);
		}
        /* if instruction window (issue buffer) full */
        if(windowHead == windowTail){
      //          cpuClock++;
       //         cpuClock_interval++;
                reordered = 0;

	/* remove all instructions which are done from beginning of window, 
	 *                  * until an instruction comes along which is not ready yet:
	 *                                   * -> check executionProfile to see which instructions are done
	 *                                                    * -> commit maximum win_size instructions (i.e. stop when issue buffer is empty)
	 *                                                                     */
	while((executionProfile[windowHead] < cpuClock) && (reordered < 4)) {
          //              cout << reordered << ", ";
			windowHead = (windowHead + 1) % win_size_const;
                        reordered++;
                	}
      
		}
	#endif

	#ifdef outoforder
        UINT32 reordered;

        /* set issue time for tail of instruction window */
        executionProfile[windowTail] = issueTime;
        windowTail = (windowTail + 1) % win_size_const;
        /* if instruction window (issue buffer) full */
        if(windowHead == windowTail){
                cpuClock++;
                cpuClock_interval++;
                reordered = 0;
                /* remove all instructions which are done from beginning of window, 
 *                  * until an instruction comes along which is not ready yet:
 *                                   * -> check executionProfile to see which instructions are done
 *                                                    * -> commit maximum win_size instructions (i.e. stop when issue buffer is empty)
 *                                                                     */
                while((executionProfile[windowHead] < cpuClock) && (reordered < 8)) {
                        windowHead = (windowHead + 1) % win_size_const;
                        reordered++;
                }
	cout << cpuClock << " : " ;
	for (int y=0;y<win_size;y++)
                cout << executionProfile[y] << ",";
        cout << endl;
                //assert(reordered != 0);
            }
                 /* reset issue times */
           issueTime = 0;

	#endif

	
	#ifdef myoutoforder
	issue_ptr = 0;
	int i=0;
		if (win_ptr == win_size_const)
			{
	//cout << cpuClock << " : " ;
				cpuClock++;
				cpuClock_interval++;
			//sort(executionProfile[0],executionprofile[win_size_consr-1]);
	/*cout << "Before: ";
	for (int x=0;x<win_size;x++)
		cout << executionProfile[x] << ",";
	cout << endl;*/
				bubble(executionProfile,win_size_const);
	/*cout << "	After: ";
	for (int y=0;y<win_size;y++)
		cout << executionProfile[y] << ",";
	cout << endl;*/
			while((executionProfile[i] < cpuClock) && (issue_ptr < win_size_const))
				{
					issue_ptr++;
					win_ptr--;
					i++;
				}
			for(int j=0; j<win_ptr;j++)
			     executionProfile[j]=executionProfile[j+issue_ptr];
			}
	executionProfile[win_ptr]= issueTime;
	win_ptr++;
	
	#endif

 /* reset issue times */
        issueTime = 0;
	}

void bubble(UINT64 *p,UINT32 N)
{
    int i, j, t;
    for (i = N-1; i >= 0; i--)
    {
        for (j = 1; j <= i; j++)
        {
            if (compare(&p[j-1], &p[j]))
            {
                t = p[j-1];
                p[j-1] = p[j];
                p[j] = t;
            }
        }
    }
}

UINT64 compare(UINT64 *m, UINT64 *n)
{
    return (*m > *n);
}

void checkIssueTime_one(){
	 if(cpuClock > issueTime)
         issueTime = cpuClock;
	}

void readRegOp_ilp_one(UINT32 regId){
        if(timeAvailable[regId] > issueTime)
        issueTime = timeAvailable[regId];
	}	

void readRegOp_ilp_one_fast(VOID* _e){
        ins_buffer_entry* e = (ins_buffer_entry*)_e;
        INT32 i;
        UINT32 regId;
        for(i=0; i < e->regReadCnt; i++){
                regId = (UINT32)e->regsRead[i];
                if(timeAvailable[regId] > issueTime)
                        issueTime = timeAvailable[regId];
        	}
	}

void writeRegOp_ilp_one(UINT32 regId){
        timeAvailable[regId] = issueTime + 1;
	}

void writeRegOp_ilp_one_fast(VOID* _e){
        ins_buffer_entry* e = (ins_buffer_entry*)_e;
        INT32 i;
        for(i=0; i < e->regWriteCnt; i++)
                timeAvailable[(UINT32)e->regsWritten[i]] = issueTime + 1;
	}

void readMem_ilp_one(ADDRINT effAddr, ADDRINT size){
        ADDRINT a;
        ADDRINT upperMemAddr, indexInChunk;
        memNode* chunk = (memNode*)NULL;
        ADDRINT shiftedAddr = effAddr >> ilp_block_size;
        ADDRINT shiftedEndAddr = (effAddr + size - 1) >> ilp_block_size;
        if(size > 0){
                for(a = shiftedAddr; a <= shiftedEndAddr; a++){
                        upperMemAddr = a >> LOG_MAX_MEM_ENTRIES;
                        indexInChunk = a ^ (upperMemAddr << LOG_MAX_MEM_ENTRIES);
                        chunk = lookup(memAddressesTable, upperMemAddr);
                        if(chunk == (memNode*)NULL){
                                chunk = install(memAddressesTable, upperMemAddr);
				all_times[chunk->timeAvailable[indexInChunk]] = issueTime;
				}
			if(all_times[chunk->timeAvailable[indexInChunk]] > issueTime)
                                issueTime = all_times[chunk->timeAvailable[indexInChunk]];
                	}
        	}
	}

void writeMem_ilp_one(ADDRINT effAddr, ADDRINT size){

        ADDRINT a;
        ADDRINT upperMemAddr, indexInChunk;
        memNode* chunk = (memNode*)NULL;
        ADDRINT shiftedAddr = effAddr >> ilp_block_size;
        ADDRINT shiftedEndAddr = (effAddr + size - 1) >> ilp_block_size;

        if(size > 0){
                for(a = shiftedAddr; a <= shiftedEndAddr; a++){
                        upperMemAddr = a >> LOG_MAX_MEM_ENTRIES;
                        indexInChunk = a ^ (upperMemAddr << LOG_MAX_MEM_ENTRIES);

                        chunk = lookup(memAddressesTable,upperMemAddr);
                        if(chunk == (memNode*)NULL)
                                chunk = install(memAddressesTable,upperMemAddr);
			if(chunk->timeAvailable[indexInChunk] == 0){
                                index_all_times++;
                                if(index_all_times >= (1 << size_pow_times))
                                        increase_size_all_times_one();
                                chunk->timeAvailable[indexInChunk] = index_all_times;
                        }
			all_times[chunk->timeAvailable[indexInChunk]] = issueTime + 1;
			}
		}
	}




void increase_size_all_times_one(){
        UINT64* ptr;

        size_pow_times++;

        ptr = (UINT64*)realloc(all_times, (1 << size_pow_times)*sizeof(UINT64));
        if(ptr == (UINT64*)NULL){
                cerr << "Could not allocate memory (realloc)!" << endl;
                exit(1);
        }
        all_times = ptr;
}
};
#endif
