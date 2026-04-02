wire "${CLK_NAME}";
wire "${CLK_NAME}"_pair = 0;

CLOCK_GENERATOR
#(
	.CLK_PERIOD("${CLK_PERIOD}")
)
i_gen_"${CLK_NAME}"
(
	.clk("${CLK_NAME}"),
	.rstnn(),
	.rst()
);
