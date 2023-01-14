// Verilog Test Fixture Template

`timescale 1ns/1ns

module testbench;
   
reg [7:0]	csabus;
reg ICLK;
reg SCLK;
reg PCLK;
reg WriteN;
wire CmdN;
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

wire	SR_SPI_Clk	= UUT.spi_shift_out.SPI_Clk;
wire	[7:0]	SR_SPI_Data_In	= UUT.spi_shift_out.SPI_Data_In;
wire	SR_SPI_Data_RdyN	= UUT.spi_shift_out.SPI_Data_RdyN;
wire	SR_SPI_Sdo	= UUT.spi_shift_out.SPI_Sdo;
wire	[7:0]	SR_SPI_shift_reg	= UUT.spi_shift_out.SPI_shift_reg;
wire	[2:0]	SR_SPI_counter	= UUT.spi_shift_out.SPI_counter;
wire	SR_SPI_DataAck	= UUT.spi_shift_out.SPI_DataAck;
wire	SR_SPI_WriteInhibit	= UUT.spi_shift_out.SPI_WriteInhibit;
wire	SR_SPI_LoadN	= UUT.spi_shift_out.SPI_LoadN;


initial 
begin
	// --------------------
		ICLK = 1;
		SCLK = 1;
		PCLK = 1;
		WriteN = 1;
		DI = 1;
		CD = 0;
		ResetN = 0;
		csabus = 8'b00000000;
		// --------------------
		#3320
		ResetN = 1;
		
		
		// Test the Address Controller functions
		#3320
		// Select Address Bank 0
		csabus = 8'b00000000;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#332
		// Write 0x55 to Address [7-0]
		csabus = 8'b01010101;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#3320
		// Select Address Bank 1
		csabus = 8'b00000001;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#332
		// Write 0xAA to Address [15-8]
		csabus = 8'b10101010;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#3320
		// Select Address Bank 2
		csabus = 8'b00000010;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#332
		// Write 0x01 to Address [16]
		csabus = 8'b00000001;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#3320
		// Select Address Bank 3 (ignored)
		csabus = 8'b00000011;
		#332
		PCLK = 0;
		#332
		PCLK = 1;
		#332
		// Write 0x69 to be ignored
		csabus = 8'b01101001;
		#332
		PCLK = 0;
		#332
		PCLK = 1;

		// Continue testing the serial output
		// until end of simulation.
		csabus = 8'b00000001;
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
			while( CmdN )
			begin
				#332
				WriteN = 0;
			end
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
			while( CmdN )
			begin
				#332
				WriteN = 0;
			end
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
			while( CmdN )
			begin
				#332
				WriteN = 0;
			end
			WriteN = 1;
			#996
			#332
			csabus = csabus + 1;
			#332
			csabus = csabus + 1;
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
			while( CmdN )
			begin
				#332
				WriteN = 0;
			end
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