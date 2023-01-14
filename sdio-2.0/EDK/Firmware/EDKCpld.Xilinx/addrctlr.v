/////////////////////////////////////////////////////////////////////////////
//
// SDIO CPLD ADDRCTLR.v Src File
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
//	Filename:	ADDRCTLR.v
//	Author:		Geoff Richmond
//	
//	Description:	Configurable SDIO Address Controller Module.
//					The Address width is configurable.
//
////////////////////////////////////////////////////////////////////////


`define	AWIDTH		17		// Width of the output Address Bus
`define	DWIDTH		8		// Width of the Data In Bus to load ADDR
`define	BSWIDTH		3		// Bank Select Width (bits necessary to 
									// select each address controller register)


module ADDR_CTLR(	ACTL_StrbN,
					ACTL_RstN,
					ACTL_Data_In,
					ACTL_Inc,
					ACTL_BSel,
				
					ACTL_Addr_Out
				);
			
// Inputs
input 				ACTL_StrbN;		// Address latch strobe
input					ACTL_RstN;		// Active low Reset signal
input [(`DWIDTH-1):0]	ACTL_Data_In;	// Data into ADDR_CTLR
input					ACTL_Inc;		// Increment Address signal
input [(`BSWIDTH-1):0]	ACTL_BSel;		// Bank Select

// Outputs
output [(`AWIDTH-1):0]	ACTL_Addr_Out;	// Address bus out

// Combinational Logic
reg [(`AWIDTH-1):0]		ACTL_Addr_Out;		// Address register


// Clocked Logic

// Control circuitry for Address Controller. If reset 
// is asserted, the address latch is set to 0. The address latch
// is loaded in blocks of the input bus width. The falling edge  
// of the strobe latches the data into the address register
// selected by Bank Select. If IncN is asserted, the address is
// incremented with each falling edge of StrbN.
always @(negedge ACTL_StrbN or negedge ACTL_RstN)
begin
	
	if(!ACTL_RstN) begin
		ACTL_Addr_Out	<= 0;
	end
	
	else begin
		
			// Address increment
			if(ACTL_Inc)
				ACTL_Addr_Out	<= ACTL_Addr_Out + 1;

			// Load Address Bank 0-2
			if(ACTL_BSel == 3'b000)
				ACTL_Addr_Out[7:0]	<= ACTL_Data_In;
				
			else if(ACTL_BSel == 3'b001)
				ACTL_Addr_Out[15:8]	<= ACTL_Data_In;
				
			else if(ACTL_BSel == 3'b010)
				ACTL_Addr_Out[16]	<= ACTL_Data_In[0];

	end
end


endmodule // ADDR_CTLR



