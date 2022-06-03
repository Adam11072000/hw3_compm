/* 046267 Computer Architecture - Winter 20/21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <vector>

using std::vector;

const int UNDEFINED = -1;

class Dependency{
public:
    int first_inst_idx;
    int second_inst_idx;

    Dependency(): first_inst_idx(UNDEFINED), second_inst_idx(UNDEFINED){}
    ~Dependency() = default;
    Dependency(const Dependency& src) = default;
};


class Simulator{
    vector<Dependency> dependencies;
    unsigned int num_of_insts;
    vector<InstInfo> prog_trace;
    vector<unsigned int> ops_latency;
    vector<int> inst_depth_in_cycles;
public:
    Simulator(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts);
    ~Simulator() = default;
    Simulator(const Simulator& src) = default;
    int get_inst_depth(unsigned int inst);
    int get_inst_depends(unsigned int inst, int* inst_1, int* inst_2);
    int get_prog_depth();
};

Simulator::Simulator(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts)
:dependencies(vector<Dependency>(numOfInsts)), num_of_insts(numOfInsts), prog_trace(vector<InstInfo>(numOfInsts)),
ops_latency(vector<unsigned int>(MAX_OPS)), inst_depth_in_cycles(vector<int>(numOfInsts))
{
    for(int i = 0; i < num_of_insts; i++){
        prog_trace[i] = progTrace[i];
        inst_depth_in_cycles[i] = 0;
    }
    for(int i = 0; i < MAX_OPS; i++){
        ops_latency[i] = opsLatency[i];
    }
    // prepare dependencies
    for(int i = 0; i < num_of_insts; i++){
        // each i iteration will provide us with instructions that inst i depends on.
        for(int j = 0; j < i; j++){
            if(progTrace[j].dstIdx == progTrace[i].src1Idx){
                dependencies[i].first_inst_idx = j;

            }
            if(progTrace[j].dstIdx == progTrace[i].src2Idx){
                dependencies[i].second_inst_idx = j;
            }
        }
        if(dependencies[i].first_inst_idx == UNDEFINED && dependencies[i].second_inst_idx == UNDEFINED){
            inst_depth_in_cycles[i] = 0;
        }else if(dependencies[i].first_inst_idx != UNDEFINED && dependencies[i].second_inst_idx == UNDEFINED){
            inst_depth_in_cycles[i] = inst_depth_in_cycles[dependencies[i].first_inst_idx]
                    + ops_latency[prog_trace[dependencies[i].first_inst_idx].opcode];
        }else if(dependencies[i].first_inst_idx == UNDEFINED && dependencies[i].second_inst_idx != UNDEFINED){
            inst_depth_in_cycles[i] = inst_depth_in_cycles[dependencies[i].second_inst_idx]
                    + ops_latency[progTrace[dependencies[i].second_inst_idx].opcode];
        }else{
            if(inst_depth_in_cycles[dependencies[i].first_inst_idx] + ops_latency[prog_trace[dependencies[i].first_inst_idx].opcode] >
                inst_depth_in_cycles[dependencies[i].second_inst_idx] + ops_latency[progTrace[dependencies[i].second_inst_idx].opcode]){
                inst_depth_in_cycles[i] = inst_depth_in_cycles[dependencies[i].first_inst_idx]
                        + ops_latency[prog_trace[dependencies[i].first_inst_idx].opcode];
            }else{
                inst_depth_in_cycles[i] = inst_depth_in_cycles[dependencies[i].second_inst_idx]
                        + ops_latency[progTrace[dependencies[i].second_inst_idx].opcode];
            }
        }
        // this loop ensures we are only dependent on 2 instructions at most, because we only take the latest.
        // since we only change inst_depth_in_cycles[i][ once for the entire loop, we make sure we get the maximum depth.
    }
}

int Simulator::get_inst_depth(unsigned int inst) {
    if(inst >= num_of_insts || inst < 0){
        return -1;
    }
    return inst_depth_in_cycles[inst];
}

int Simulator::get_inst_depends(unsigned int inst, int *inst_1, int *inst_2) {
    if(inst >= num_of_insts || inst < 0){
        return -1;
    }
    *inst_1 = dependencies[inst].first_inst_idx;
    *inst_2 = dependencies[inst].second_inst_idx;
    return 0;
}

int Simulator::get_prog_depth() {
    int max = UNDEFINED;
    for(int i = 0; i < num_of_insts; i++){
        if((int) max < (int) (inst_depth_in_cycles[i] + ops_latency[prog_trace[i].opcode])){
            max = inst_depth_in_cycles[i] + ops_latency[prog_trace[i].opcode];
        }
    }
    return max;
}


ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    Simulator* sim = nullptr;
    try {
        sim = new Simulator(opsLatency, progTrace, numOfInsts);
    }catch (std::bad_alloc&){
        return PROG_CTX_NULL;
    }
    ProgCtx to_return = (void*) sim;
    return to_return;
}

void freeProgCtx(ProgCtx ctx) {
    Simulator* to_delete = (Simulator*) ctx;
    delete to_delete;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    Simulator* sim = (Simulator*) ctx;
    return sim->get_inst_depth(theInst);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    Simulator* sim = (Simulator*) ctx;
    return sim->get_inst_depends(theInst, src1DepInst, src2DepInst);
}

int getProgDepth(ProgCtx ctx){
    Simulator* sim = (Simulator*) ctx;
    return sim->get_prog_depth();
}


