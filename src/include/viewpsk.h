// ----------------------------------------------------------------------------
// psk.h  --  psk modem
//
// Copyright (C) 2006
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.  Adapted from code contained in gmfsk source code 
// distribution.
//
// fldigi is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ----------------------------------------------------------------------------

#ifndef _VIEWPSK_H
#define _VIEWPSK_H

#include "complex.h"
#include "trx.h"
#include "filters.h"

//=====================================================================
#define	VPSKSAMPLERATE	(8000)
#define VSNTHRESHOLD 2.0
#define VAFCDECAY 8
#define CHANNELS 25
#define VSEARCHWIDTH 50
#define VSIGSEARCH 3
#define VWAITCOUNT 4
//=====================================================================

class viewpsk {
private:
	int				symbollen;
	double			phaseacc[CHANNELS];
	complex			prevsymbol[CHANNELS];
	complex			quality[CHANNELS];
	unsigned int	shreg[CHANNELS];
	double			metric[CHANNELS];
	
	double			frequency[CHANNELS];
	int				nomfreq[CHANNELS];
	double			freqerr[CHANNELS];
	double			phase[CHANNELS];
	double			bandwidth;

	C_FIR_filter	*fir1[CHANNELS];
	C_FIR_filter	*fir2[CHANNELS];

	int				bits[CHANNELS];
	double 			bitclk[CHANNELS];
	double 			syncbuf[CHANNELS * 16];
	unsigned int	dcdshreg[CHANNELS];
	int 			dcd[CHANNELS];
	int				dcdbits;
	int				sigsearch[CHANNELS];
	int				waitcount[CHANNELS];

	void			rx_symbol(int ch, complex symbol);
	void 			rx_bit(int ch, int bit);

	void			findsignal(int);
	void			afc(int);
	
public:
	viewpsk(trx_mode mode);
	~viewpsk();
	void init();
	void restart(trx_mode mode);
	void rx_init(){};
	void tx_init(cSound *sc){};
	void restart() {};
	int rx_process(const double *buf, int len);
	int get_freq(int n) { return (int)frequency[n];}
};

#endif
