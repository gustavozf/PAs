#include "base/intmath.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "debug/Fetch.hh"


// Inicializacao
PAs::PAs(const PAsParams *params)
        : PAs(params),
        localPredictorSize(params->localPredictorSize), // Para o "P"
        localCtrBits(params->localCtrBits),
        localHistoryTableSize(params->localHistoryTableSize),
        localHistoryBits(ceilLog2(params->localPredictorSize)),
        globalPredictorSize(params->globalPredictorSize), // Para o "s"
        globalCtrBits(params->globalCtrBits),
        globalHistory(params->numThreads, 0),
        globalHistoryBits(
        ceilLog2(params->globalPredictorSize) >
        ceilLog2(params->choicePredictorSize) ?
        ceilLog2(params->globalPredictorSize) :
        ceilLog2(params->choicePredictorSize))
{


}

// Update: Atualiza a tabela de predicao
void
PAs::update(ThreadID tid, Addr branch_addr, bool taken,
                     void *bp_history, bool squashed)
{

}

// Lookup: decide se vai ter salto ou nao
bool
PAs::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{

}


void
PAs::squash(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory *>(bp_history);

    // Restore global history to state prior to this branch.
    globalHistory[tid] = history->globalHistory;

    // Restore local history
    if (history->localHistoryIdx != invalidPredictorIndex) {
        localHistoryTable[history->localHistoryIdx] = history->localHistory;
    }

    // Delete this BPHistory now that we're done with it.
    delete history;
}

void
TournamentBP::uncondBranch(ThreadID tid, Addr pc, void * &bp_history)
{
    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->globalHistory = globalHistory[tid];
    history->localPredTaken = true;
    history->globalPredTaken = true;
    history->globalUsed = true;
    history->localHistoryIdx = invalidPredictorIndex;
    history->localHistory = invalidPredictorIndex;
    bp_history = static_cast<void *>(history);

    //updateGlobalHistTaken(tid);
}

PAs*
PAsParams::create()
{
    return new PAs(this);
}