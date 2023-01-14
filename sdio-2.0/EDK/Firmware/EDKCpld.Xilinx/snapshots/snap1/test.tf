// Verilog Test Fixture Template

`timescale 1ns/1ns

module testbench;
   
reg [7:0]	csabus;
reg ICLK;
reg SCLK;
reg PCLK;
reg WriteN;
reg CmdN;
reg DI;
reg CD;
reg ResetN;

wire RdyN;
wire [16:0]		addr;
wire DO;

SDIO	UUT(	.csabus(csabus),
				.ICLK(ICLK),
				.SCLK(SCLK),
				.PCLK(PCLK),
				.WriteN(WriteN),
				.RdyN(RdyN),
				.CmdN(CmdN),
				.DI(DI),
				.DO(DO),
				.CD(CD),
				.addr(addr),
				.ResetN(ResetN) );

// Display internal signals in simulation
wire	[7:0]	FifoData_In	= UUT.fifo.FIFO_Data_In;
wire	FifoFWriteN	= UUT.fifo.FIFO_WriteN;
wire	FifoClk	= UUT.fifo.FIFO_Clk;
wire	[7:0]	FifoFIFO_Data_Out	= UUT.fifo.FIFO_Data_Out;
wire	FifoF_FIFO_RcvrRdy	= UUT.fifo.FIFO_RcvrRdy;
wire	FifoF_FIFO_RcvrLoad	= UUT.fifo.FIFO_RcvrLoading;
wire	FifoF_DataRdyN = UUT.fifo.FIFO_DataRdyN;
wire	FifoRstN	= UUT.fifo.FIFO_RstN;
wire	FifoF_Full 	= UUT.fifo.FIFO_Full;

wire	SR_SPI_Clk	= UUT.spi_shift_out.SPI_Clk;
wire	SR_SPI_DataRdy	= UUT.spi_shift_out.SPI_DataRdy;
wire	[7:0]	SR_SPI_Data_In	= UUT.spi_shift_out.SPI_Data_In;
wire	SR_SPI_Sdo	= UUT.spi_shift_out.SPI_Sdo;
wire	SR_SPI_EmptyN	= UUT.spi_shift_out.SPI_EmptyN;
wire	[7:0]	SR_SPI_shift_reg	= UUT.spi_shift_out.SPI_shift_reg;
wire	[2:0]	SR_SPI_counter	= UUT.spi_shift_out.SPI_counter;


initial 
begin
	// --------------------
		ICLK = 1;
		SCLK = 1;
		PCLK = 1;
		WriteN = 1;
		CmdN = 1;
		DI = 0;
		CD = 0;
		ResetN = 0;
		csabus = 8'b00000000;
		// --------------------
		#4000
		ResetN = 1;
		
		
		// Set mode to output on SPI bus
		#1000
		CmdN = 0;
		csabus = 8'b00100000;
		#2000
		CmdN = 1;
		csabus = 8'b00000000;


		// Write 16 bytes to SPI Bus.
		#5000
			while( RdyN )
			begin
				#332
				WriteN = 1;
			end
			#332
			csabus = csabus + 1;
			#332
			WriteN = 0;
			#332
			WriteN = 1;
			#996
			while( RdyN )
			begin
				#332
				WriteN = 1;
			end
			#332
			csabus = csabus + 1;
			#332
			WriteN = 0;
			#332
			WriteN = 1;
			#332
			while( RdyN )
			begin
				#332
				WriteN = 1;
			end
			#332
			csabus = csabus + 1;
			#332
			WriteN = 0;
			#332
			WriteN = 1;
			#996
		#60000
		while( 1 )
		begin
			while( RdyN )
			begin
				#332
				WriteN = 1;
			end
			#332
			csabus = csabus + 1;
			#332
			WriteN = 0;
			#332
			WriteN = 1;
		end
		// --------------------
		#256000
		$finish;

end

// Instruction clock is a constant 1 MHz
always
begin
	#166	ICLK = ~ICLK;
end

// SPI clock is a constant 512 kHz
always
begin
	#977	SCLK = ~SCLK;
end

endmodule