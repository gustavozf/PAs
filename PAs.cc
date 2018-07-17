/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Anthony Gutierrez
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#include "cpu/pred/PAs.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

PAsBP::PAsBP(const PAsBPParams *params)
    : BPredUnit(params),
      k(params->k),
      a(params->a),
      m(params->m),
      bitsSPHT(params->tamPred),
      bitsPBHT(params->tamHistBHT)

    tamPBHT = pow(2,a);
    numColSPHT = pow(2, m);
    numLinSPHT = pow(2, k);

    PBHT.resize(tamPBHT);
    for(int i=0; i < tamPBHT; ++i){
        PBHT[i] = 0;
    }

    SPHT.resize(numLinSPHT);
    for(int i = 0; i < numLinSPHT; ++i){
        SPHT[i].resize(numColSPHT);
        for(int j=0; j < numColSPHT; ++j){
            SPHT[i][j].setBits(bitsSPHT);
        }
    }
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
PAsBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    unsigned index = (branchAddr & tamPBHT);
    unsigned linhaSPHT = (PBHT[indexPBHT] & numLinSPHT);
    unsigned colunaSPHT = (branchAddr & numColPBHT);

    PBHT[index] = (PBHT[index] << 1);

    PBHT[index] |=  1;
    SPHT[linhaSPHT][colunaSPHT].increment();
}

// Apaga o historico
void
PAsBP::squash(ThreadID tid, void *bpHistory)
{
    //BPHistory *history = static_cast<BPHistory*>(bpHistory);
    //globalHistoryReg[tid] = history->globalHistoryReg;

    //delete history;
}

bool
PAsBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    unsigned indexPBHT = (branchAddr & tamPBHT);
    unsigned linhaSPHT = (PBHT[indexPBHT] & numLinSPHT);
    unsigned colunaSPHT = (branchAddr & numColPBHT);
    
    bool finalPrediction = (SPHT[linhaSPHT][colunaSPHT] > 1);

    return finalPrediction;
}

// So no primeiro nivel
void
PAsBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    
}

void
PAsBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    if (squashed) {
        //globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        return;
    }

    unsigned index = (branchAddr & tamPBHT);
    unsigned linhaSPHT = (PBHT[indexPBHT] & numLinSPHT);
    unsigned colunaSPHT = (branchAddr & numColPBHT);

    PBHT[index] = (PBHT[index] << 1);

    if (taken){
        PBHT[index] |=  1;
        SPHT[linhaSPHT][colunaSPHT].increment();
    } else{
        SPHT[linhaSPHT][colunaSPHT].decrement();
    }
}

PAsBP*
PAsBPParams::create()
{
    return new PAsBP(this);
}
