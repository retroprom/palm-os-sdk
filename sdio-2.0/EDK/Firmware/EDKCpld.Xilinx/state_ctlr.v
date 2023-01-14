/////////////////////////////////////////////////////////////////////////////
//
// SDIO CPLD STATE_CTLR.v Src File
//
// Copyright (c) 2001 Palm Inc. or its subsidiaries.
// All rights reserved.  This software may be copied and used solely for 
// developing products for the communicating with the Palm Inc. platform
// and for archival and backup purposes.  Except for the foregoing, no part 
// of this software may be reproduced or transmitted in any form or by any 
// means or used to make any derivative work (such as translation, 
// transformation or adaptation) without express written consent from Palm Inc.
//
// Palm Inc. reserves the right to revise this software and to make changes in 
// content from time to time without obligation on the part of Palm Inc. to 
// provide notification of such revision or changes.  PALM INC. MAKES NO 
// REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF ERRORS OR 
// THAT THE SOFTWARE IS SUITABLE FOR YOUR USE.  THE SOFTWARE IS PROVIDED ON
// AN "AS IS" BASIS.  PALM INC. MAKES NO WARRANTIES, TERMS OR CONDITIONS, EXPRESS
// OR IMPLIED, EITHER IN FACT OR BY OPERATION OF LAW, STATUTORY OR 
// OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
//
// TO THE FULL EXTENT ALLOWED BY LAW, PALM INC. ALSO EXCLUDES FOR ITSELF AND ITS
// SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING 
// NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
// PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
// BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING 
// OUT OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF PALM INC. HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES.
//
// HotSync, Palm Inc., and Graffiti are registered trademarks, 
// and Palm III and Palm OS are trademarks of Palm Inc. or its 
// subsidiaries.
//
// IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK, THE OTHER SOFTWARE AND 
// DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT 
// ACCOMPANYING THE COMPACT DISK.
// 
/////////////////////////////////////////////////////////////////////////////
//
// Revision History:
//
// Date         Name        Description
// ----------------------------------------------------------
// 12/20/01		GAR         initial release
//
/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	Filename:	STATE_CTLR.v
//	Author:		Geoff Richmond
//	
//	Description:	SDIO CPLD Controller Module.
//
////////////////////////////////////////////////////////////////////////

`define	DWIDTH		8		// Width of the Data In Bus to load ADDR
`define	BSWIDTH		3		// Bank Select Width (bits necessary to 
									// select each address controller register)

module STATE_CTLR(	SC_IClk,
					SC_PClk,
					SC_ResetN,
					SC_Data_Bus,
					SC_StrbN,
					SC_BSel,
					SC_Addr_Inc
				);
			
// Inputs
input					SC_IClk;			// Instruction Clock
input					SC_PClk;			// Port Clock
input					SC_ResetN;		// Active low Reset signal
input	[(`DWIDTH-1):0]			SC_Data_Bus;		// Data Bus

// Outputs
output 				SC_StrbN;		// Address latch strobe
output [(`BSWIDTH-1):0]	SC_BSel;	// Bank Select
output				SC_Addr_Inc;	// Increment Address signal

// Status Register.
//		Bit 6		- Address increment control
//		Bit 4		- Addr Ctlr Strb Select
//		Bit 3		- Addr Ctlr Strb Enable
//		Bit 2		- Bank Select 2
//		Bit 1		- Bank Select 1
//		Bit 0		- Bank Select 0
//
`define	STATBITS		8		// Number of bits in status register - 8 bits
reg [(`DWIDTH-1):0]	SC_Status;
reg						SC_StateLoadN;			// State register load control

assign					SC_BSel[2]	= SC_Status[2];
assign					SC_BSel[1]	= SC_Status[1];
assign					SC_BSel[0]	= SC_Status[0];


// Combinational Logic

reg						SC_StrbN;
wire [(`BSWIDTH-1):0]	SC_BSel;
wire						SC_Addr_Inc;

// Address Controller controls
assign					SC_Addr_Inc = SC_Status[6];


// Clocked Logic

// Control circuitry for State Controller. If reset 
// is asserted, clear the status register. 

always @(negedge SC_PClk or negedge SC_ResetN) begin

	if(!SC_ResetN) begin
		SC_Status		<= 0;
		SC_StateLoadN	<= 0;
	end

	else begin
		if( ~SC_StateLoadN ) begin
			// Load the status register
			SC_Status[(`STATBITS-1):0]	<= SC_Data_Bus[(`STATBITS-1):0];
			SC_StrbN	<= 1;
			SC_StateLoadN	<= 1;
		end
		else
		begin
			// Strobe the Address Controller
			SC_StrbN	<= 0;
			SC_StateLoadN	<= 0;
		end
	end

end

endmodule // STATE_CTLR



