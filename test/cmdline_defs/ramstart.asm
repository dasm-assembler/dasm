
    .PROCESSOR	68HC11

    .ifnconst	RAM_START
RAM_START	.EQU		0x80
    .endif

    ORG		0

    DC.w		RAM_START


