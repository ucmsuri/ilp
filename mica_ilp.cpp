/* 
 * This file is part of MICA, a Pin tool to collect
 * microarchitecture-independent program characteristics using the Pin
 * instrumentation framework. 
 *
 * Please see the README.txt file distributed with the MICA release for more
 * information.
 */

#include "pin.H"

/* MICA includes */
#include "mica_utils.h"
#include "mica_ilp.h"

#include <iostream>
using namespace std;

//#define ILP_WIN_SIZE_CNT 4

//const UINT32 win_sizes[ILP_WIN_SIZE_CNT] = {32, 64, 128, 256};
#define MAX_THREAD 5

my_ilp ilp[MAX_THREAD];

//extern UINT32 _ilp_win_size;
//UINT32 win_size;

//extern UINT32 _block_size;
//UINT32 ilp_block_size;

extern INT32 numThreads;

/* buffer settings */

//#define ILP_BUFFER_SIZE 256

void init_ilp_buffering();
VOID fini_ilp_buffering_all();
VOID fini_ilp_buffering_one(THREADID tid);

/* Global variables */

ofstream output_file_ilp_one;
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "ilp_win_32_1_1.out", "specify output file name");
//KNOB<string> KnobissueFile(KNOB_MODE_WRITEONCE, "pintool", "o", "is_file.out", "specify output file name");
/*INT32 size_pow_all_times_all;
INT64 index_all_times_all;
UINT64* all_times_all[ILP_WIN_SIZE_CNT];

INT32 size_pow_times;
INT64 index_all_times;
UINT64* all_times;

INT64 cpuClock_interval_all[ILP_WIN_SIZE_CNT];
UINT64 timeAvailable_all[ILP_WIN_SIZE_CNT][MAX_NUM_REGS];
nlist* memAddressesTable_all[MAX_MEM_TABLE_ENTRIES];
UINT32 windowHead_all[ILP_WIN_SIZE_CNT];
UINT32 windowTail_all[ILP_WIN_SIZE_CNT];
UINT64 cpuClock_all[ILP_WIN_SIZE_CNT];
UINT64* executionProfile_all[ILP_WIN_SIZE_CNT];
UINT64 issueTime_all[ILP_WIN_SIZE_CNT];

INT64 cpuClock_interval;
UINT64 timeAvailable[MAX_NUM_REGS];
nlist* memAddressesTable[MAX_MEM_TABLE_ENTRIES];
UINT32 windowHead;
UINT32 windowTail;
UINT64 cpuClock;
UINT64* executionProfile;
UINT64 issueTime;
*/
/*************************
      ILP (COMMON)
**************************/

/* initializing */
//void init_ilp_common(){
	/* initializing total instruction counts is done in mica.cpp */
//}

/************************************
     ILP (one given window size) 
*************************************/

/* initializing */
/*void init_ilp_one(){

	UINT32 i;
	//char filename[100];

	init_ilp_common();
	//init_ilp_buffering();

	win_size = _ilp_win_size;
        ilp_block_size = _block_size;

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

}*/

/* support 
void increase_size_all_times_one(){
	UINT64* ptr;

	size_pow_times++;

	ptr = (UINT64*)realloc(all_times, (1 << size_pow_times)*sizeof(UINT64));
	if(ptr == (UINT64*)NULL){
		cerr << "Could not allocate memory (realloc)!" << endl;
		exit(1);
	}
	all_times = ptr;
}*/

/* per-instruction stuff */
/*VOID ilp_instr_one(){

	const UINT32 win_size_const = win_size;
	UINT32 reordered;

	// set issue time for tail of instruction window 
	executionProfile[windowTail] = issueTime;
	windowTail = (windowTail + 1) % win_size_const;

	// if instruction window (issue buffer) full 
	if(windowHead == windowTail){
		cpuClock++;
		cpuClock_interval++;
		reordered = 0;
		// remove all instructions which are done from beginning of window, 
		 // until an instruction comes along which is not ready yet:
		 // -> check executionProfile to see which instructions are done
		 // -> commit maximum win_size instructions (i.e. stop when issue buffer is empty)
		 
		while((executionProfile[windowHead] < cpuClock) && (reordered < win_size_const)) {
			windowHead = (windowHead + 1) % win_size_const;
			reordered++;
		}
		//assert(reordered != 0);
	}

	// reset issue times 
	issueTime = 0;
}

VOID ilp_instr_full_one(){
	
	// counting instructions is done in all_instr_full() 
	
	ilp_instr_one();
}*/


/*VOID checkIssueTime_one(){

	if(cpuClock > issueTime)
		issueTime = cpuClock;
}

// register stuff 
VOID readRegOp_ilp_one(UINT32 regId){

	if(timeAvailable[regId] > issueTime)
		issueTime = timeAvailable[regId];
}

VOID readRegOp_ilp_one_fast(VOID* _e){

	ins_buffer_entry* e = (ins_buffer_entry*)_e;

	INT32 i;

	UINT32 regId;

	for(i=0; i < e->regReadCnt; i++){
		regId = (UINT32)e->regsRead[i];
		if(timeAvailable[regId] > issueTime)
			issueTime = timeAvailable[regId];
	}
}

VOID writeRegOp_ilp_one(UINT32 regId){

	timeAvailable[regId] = issueTime + 1;
}

VOID writeRegOp_ilp_one_fast(VOID* _e){

	ins_buffer_entry* e = (ins_buffer_entry*)_e;

	INT32 i;

	for(i=0; i < e->regWriteCnt; i++)
		timeAvailable[(UINT32)e->regsWritten[i]] = issueTime + 1;
}

// memory access stuff 
VOID readMem_ilp_one(ADDRINT effAddr, ADDRINT size){


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
                        if(chunk == (memNode*)NULL)
                                chunk = install(memAddressesTable, upperMemAddr);

                        //assert(indexInChunk < MAX_MEM_ENTRIES);
                        //assert(chunk->timeAvailable[indexInChunk] < (1 << size_pow_times));
                        if(all_times[chunk->timeAvailable[indexInChunk]] > issueTime)
                                issueTime = all_times[chunk->timeAvailable[indexInChunk]];
                }
        }
}

VOID writeMem_ilp_one(ADDRINT effAddr, ADDRINT size){

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

                        //assert(indexInChunk < MAX_MEM_ENTRIES);
                        if(chunk->timeAvailable[indexInChunk] == 0){
                                index_all_times++;
                                if(index_all_times >= (1 << size_pow_times))
                                        increase_size_all_times_one();
                                chunk->timeAvailable[indexInChunk] = index_all_times;
                        }
                        //assert(chunk->timeAvailable[indexInChunk] < (1 << size_pow_times));
                        all_times[chunk->timeAvailable[indexInChunk]] = issueTime + 1;
                }
        }
}
*/

/* finishing... */
VOID fini_ilp_one(INT32 code, VOID* v){
	int x;
	char filename[100];

		output_file_ilp_one.open(KnobOutputFile.Value().c_str());
		//sprintf(filename,"ilp-win%d_full_int_pin.out", ilp[0].win_size);
		//output_file_ilp_one.open(filename, ios::out|ios::trunc);
for (x=0;x<numThreads;x++)
        {
	fini_ilp_buffering_one(x);
                thread_data_t* tdata = get_tls(x);
                output_file_ilp_one << tdata->total_ins_count;	
		output_file_ilp_one << " " << ilp[x].cpuClock << endl;
	}
cout.setf(ios::fixed, ios::floatfield);
   cout.setf(ios::showpoint);

for (x=0;x<numThreads;x++)
        {
        thread_data_t* tdata = get_tls(x);
	double ilp_t = (double)(tdata->total_ins_count)/ ilp[x].cpuClock;
	output_file_ilp_one << ilp_t << ", ";
	}
	output_file_ilp_one.close();
}

/***************************************
     ILP (all 4 hardcoded window sizes) 
****************************************/




/**************************
     ILP (BUFFERING)  
***************************/

/*
 * notes
 *
 * using PIN_FAST_ANALYSIS_CALL for buffering functions was tested
 * during the preparation of MICA v0.3, but showed to slightly slowdown 
 * things instead of speeding them up, so it was dropped in the end 
 */

/* initializing */
/*void init_ilp_buffering(){

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
}*/

VOID ilp_buffer_instruction_only(void* _e, THREADID tid){
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->e = (ins_buffer_entry*)_e;
}

VOID ilp_buffer_instruction_read(ADDRINT read1_addr, ADDRINT read_size, THREADID tid){
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->mem_read1_addr = read1_addr;
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->mem_read_size = read_size;
}

VOID ilp_buffer_instruction_read2(ADDRINT read2_addr, THREADID tid){
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->mem_read2_addr = read2_addr;
}

VOID ilp_buffer_instruction_write(ADDRINT write_addr, ADDRINT write_size, THREADID tid){
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->mem_write_addr = write_addr;
	ilp[tid].buff.ilp_buffer[ilp[tid].buff.ilp_buffer_index]->mem_write_size = write_size;
}

ADDRINT ilp_buffer_instruction_next( THREADID tid){
       ilp[tid].buff. ilp_buffer_index++;
        return (ADDRINT)(ilp[tid].buff.ilp_buffer_index == ILP_BUFFER_SIZE );
}

/* empty buffer for one given window size  */
VOID empty_buffer_one(THREADID tid){
	UINT32 i,j;
	thread_data_t* tdata = get_tls(tid);
        //cout << tdata->total_ins_count ;
//cout << ilp[tid].buff.ilp_buffer_index << endl;
	for(i=0; i < ilp[tid].buff.ilp_buffer_index; i++){

		// register reads 
		for(j=0; j < (UINT32)ilp[tid].buff.ilp_buffer[i]->e->regReadCnt; j++){
			ilp[tid].readRegOp_ilp_one((UINT32)ilp[tid].buff.ilp_buffer[i]->e->regsRead[j]);
		}

		// memory reads
		if(ilp[tid].buff.ilp_buffer[i]->mem_read1_addr != 0){
			ilp[tid].readMem_ilp_one(ilp[tid].buff.ilp_buffer[i]->mem_read1_addr, ilp[tid].buff.ilp_buffer[i]->mem_read_size);
			ilp[tid].buff.ilp_buffer[i]->mem_read1_addr = 0;
		
			if(ilp[tid].buff.ilp_buffer[i]->mem_read2_addr != 0){
				ilp[tid].readMem_ilp_one(ilp[tid].buff.ilp_buffer[i]->mem_read2_addr, ilp[tid].buff.ilp_buffer[i]->mem_read_size);
				ilp[tid].buff.ilp_buffer[i]->mem_read2_addr = 0;
			}

			ilp[tid].buff.ilp_buffer[i]->mem_read_size = 0;
		}

		ilp[tid].checkIssueTime_one();

		// register writes
		for(j=0; j < (UINT32)ilp[tid].buff.ilp_buffer[i]->e->regWriteCnt; j++){
			 ilp[tid].writeRegOp_ilp_one((UINT32)ilp[tid].buff.ilp_buffer[i]->e->regsWritten[j]);	
		}

		// memory writes
		if(ilp[tid].buff.ilp_buffer[i]->mem_write_addr != 0){
			ilp[tid].writeMem_ilp_one(ilp[tid].buff.ilp_buffer[i]->mem_write_addr, ilp[tid].buff.ilp_buffer[i]->mem_write_size);
			ilp[tid].buff.ilp_buffer[i]->mem_write_addr = 0;
			ilp[tid].buff.ilp_buffer[i]->mem_write_size = 0;
		}

		ilp[tid].buff.ilp_buffer[i]->e = (ins_buffer_entry*)NULL;

			ilp[tid].ilp_instr_full_one();
	}

	ilp[tid].buff.ilp_buffer_index = 0;
}

/* instrumenting (instruction level) */
VOID instrument_ilp_buffering_common(INS ins, ins_buffer_entry* e){

	UINT32 i, maxNumRegsProd, maxNumRegsCons, regReadCnt, regWriteCnt;
	REG reg;

	// buffer register reads per static instruction
	if(!e->setRead){


		// register reads and memory reads determine the issue time
		maxNumRegsCons = INS_MaxNumRRegs(ins);

		regReadCnt = 0;	
		for(i=0; i < maxNumRegsCons; i++){
			reg = INS_RegR(ins, i);
			//assert((UINT32)reg < MAX_NUM_REGS);
			// only consider valid general-purpose registers (any bit-width) and floating-point registers,
			// i.e. exlude branch, segment and pin registers, among others
			if(REG_valid(reg) && (REG_is_fr(reg) || REG_is_mm(reg) || REG_is_xmm(reg) || REG_is_gr(reg) || REG_is_gr8(reg) || REG_is_gr16(reg) || REG_is_gr32(reg) || REG_is_gr64(reg))){
				regReadCnt++;
			}
		}
		
		e->regReadCnt = regReadCnt;
		e->regsRead = (REG*)malloc(regReadCnt*sizeof(REG));
		/*if((e->regsRead = (REG*)malloc(regReadCnt*sizeof(REG))) == (REG*)NULL){
			fprintf(stderr,"ERROR: Could not allocate regsRead memory for ins 0x%x\n", (unsigned int)e->insAddr);
			exit(1);
		}*/

		regReadCnt = 0;
		for(i=0; i < maxNumRegsCons; i++){
	
			reg = INS_RegR(ins, i);

			//assert((UINT32)reg < MAX_NUM_REGS);
			// only consider valid general-purpose registers (any bit-width) and floating-point registers,
			// i.e. exlude branch, segment and pin registers, among others
			if(REG_valid(reg) && (REG_is_fr(reg) || REG_is_mm(reg) || REG_is_xmm(reg) || REG_is_gr(reg) || REG_is_gr8(reg) || REG_is_gr16(reg) || REG_is_gr32(reg) || REG_is_gr64(reg))){
				e->regsRead[regReadCnt++] = reg;
			}
		}

		e->setRead = true;
		
	}

	// buffer register writes per static instruction
	if(!e->setWritten){	
		maxNumRegsProd = INS_MaxNumWRegs(ins);

		regWriteCnt = 0;
		for(i=0; i < maxNumRegsProd; i++){

			reg = INS_RegW(ins, i);

			//assert((UINT32)reg < MAX_NUM_REGS);
			// only consider valid general-purpose registers (any bit-width) and floating-point registers,
			// i.e. exlude branch, segment and pin registers, among others */
			if(REG_valid(reg) && (REG_is_fr(reg) || REG_is_mm(reg) || REG_is_xmm(reg) || REG_is_gr(reg) || REG_is_gr8(reg) || REG_is_gr16(reg) || REG_is_gr32(reg) || REG_is_gr64(reg))){
				regWriteCnt++;
			}
		}

		e->regWriteCnt = regWriteCnt;
		e->regsWritten = (REG*)malloc(regWriteCnt*sizeof(REG));
		/*if((e->regsWritten = (REG*)malloc(regWriteCnt*sizeof(REG))) == (REG*)NULL){
			fprintf(stderr,"ERROR: Could not allocate regsRead memory for ins 0x%x\n", (unsigned int)e->insAddr);
			exit(1);
		}*/	

		regWriteCnt = 0;
		for(i=0; i < maxNumRegsProd; i++){

			reg = INS_RegW(ins, i);

			//assert((UINT32)reg < MAX_NUM_REGS);
			// only consider valid general-purpose registers (any bit-width) and floating-point registers,
			// i.e. exlude branch, segment and pin registers, among others
			if(REG_valid(reg) && (REG_is_fr(reg) || REG_is_mm(reg) || REG_is_xmm(reg) || REG_is_gr(reg) || REG_is_gr8(reg) || REG_is_gr16(reg) || REG_is_gr32(reg) || REG_is_gr64(reg))){
				e->regsWritten[regWriteCnt++] = reg;
			}
		}
	
		e->setWritten = true;
	}

	// buffer memory operations (and instruction register buffer) with one single InsertCall
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ilp_buffer_instruction_only, IARG_PTR, (void*)e, IARG_THREAD_ID,IARG_END);

        if(INS_IsMemoryRead(ins)){
				
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ilp_buffer_instruction_read, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_THREAD_ID,IARG_END);

                if(INS_HasMemoryRead2(ins)){
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ilp_buffer_instruction_read2, IARG_MEMORYREAD2_EA,IARG_THREAD_ID, IARG_END);
                }
        }
                        
        if(INS_IsMemoryWrite(ins)){
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ilp_buffer_instruction_write, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE,IARG_THREAD_ID, IARG_END);
        }
        
	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)ilp_buffer_instruction_next,IARG_THREAD_ID, IARG_END);

}

VOID instrument_ilp_one(INS ins, ins_buffer_entry* e){
	instrument_ilp_buffering_common(ins, e);
	// only called if buffer is full 
	INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)empty_buffer_one,IARG_THREAD_ID, IARG_END);
}



VOID fini_ilp_buffering_one(THREADID tid){

	if(ilp[tid].buff.ilp_buffer_index != 0)
		empty_buffer_one(tid);
}

