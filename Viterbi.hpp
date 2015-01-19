/*
 * Copyright (C) 2014  mingspy@163.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once
#include <vector>
#include "SparseInstance.hpp"
#include "Dictionary.hpp"
#include <assert.h>
using namespace std;

namespace mingspy
{
const double PROB_INFINT = 1000000000;
class Viterbi
{
public:
    static double viterbi( const vector<const WordNature *> & Observs, const NatureProbTable & shiftContext,
                           SparseInstance<int> & bestPos)
    {
        int T = Observs.size();
        Matrix<double> delta;
        Matrix<int> psi;

        // 1. Initialize.
        for(int i = 0; i< Observs[0]->numValues(); i++) {
            double emitp = -log((Observs[0]->valueAt(i) + 1.0)/ (
                                    shiftContext.getNatureTotal(Observs[0]->attrAt(i)) + 66));
            assert(emitp >= 0);
            delta[0].setAttrValue(Observs[0]->attrAt(i), emitp);
        }

        // 2. Induction.
        // Get the best path from ti-1 -> ti;
        // In HMM: delta[t][j] = Max(delta[t-1][i]*a[i][j])*b[j][Obs[t]]
        //    or  delta[t][j] = Min{-[log[delta[t-1][i]+log(a[i][j])+log(b[j][Obs[t])]}
        int index = 0;
        double minProb = PROB_INFINT;
        double proba = 0;

        for ( int t = 1; t < T; t++ ) {
            // Calculate each roles best delta from previous to current.
            for ( int j = 0; j < Observs[t]->numValues(); j++ ) {
                minProb = PROB_INFINT;
                for ( int i = 0; i < Observs[t - 1]->numValues(); i++ ) {
                    // Get a[i][j], same as -logP(ti-1|ti).
                    proba = -log(shiftContext.getCoProb(Observs[t - 1]->attrAt(i), Observs[t]->attrAt(j)));
                    // Add delta[t-1][i].
                    proba += delta[t - 1].valueAt(i);
                    if ( proba < minProb ) {
                        index = i;
                        minProb = proba;
                    }
                }
                psi[ t ].setAttrValue(j,index);
                double emitProb = -log((Observs[t]->valueAt(j) + 1.0)/ (
                                           shiftContext.getNatureTotal(Observs[t]->attrAt(j)) + 66));
                assert(emitProb >= 0);
                delta[ t ].setAttrValue(j, minProb + emitProb);
            }
        }

        // 3.Terminal.
        // Record the best role tag's index.
        //bestPos.setAttrValue(T, -1);
        minProb = PROB_INFINT;
        index = 0;
        SparseInstance<double> & lastDelta = delta[T - 1];
        for ( int i = 0; i < lastDelta.numValues(); i++ ) {
            if ( lastDelta.valueAt(i) < minProb ) {
                index = i;
                minProb = lastDelta.valueAt(i);
            }
        }

        // Get best path.
        bestPos.setAttrValue(T - 1,index);
        for ( int t = T - 1; t > 0; t-- ) {
            bestPos.setAttrValue(t - 1, psi[t].valueAt(bestPos.getAttrValue(t)));
        }

        // Get best pose.
        for ( int i = 0; i < T; i++ ) {
            bestPos.setAttrValue(i, Observs[i]->attrAt(bestPos.valueAt(i)));
        }
        assert(minProb >=0);
        return minProb;
    }
};
}

