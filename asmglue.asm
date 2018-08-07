	case	on
	mcopy	asmglue.macros

ROMIN	gequ	$E0C081
LCBANK1	gequ	$E0C08B
LCBANK2	gequ	$E0C083
STATEREG gequ	$E0C068

ForceLCBank1 start
	short	i,m
	lda	>STATEREG	;get original state reg.
	tax
	lda	>LCBANK1	;force LC bank 1
	lda	>LCBANK1
	long	i,m
	txa
	rtl
	end

ForceLCBank2 start
	short	i,m
	lda	>STATEREG	;get original state reg.
	tax
	lda	>LCBANK2	;force LC bank 2
	lda	>LCBANK2
	long	i,m
	txa
	rtl
	end

ForceRomIn start
	short	i,m
	lda	>STATEREG	;get original state reg.
	tax
	lda	>ROMIN		;force ROM in to Language Card space
	lda	>ROMIN
	long	i,m
	txa
	rtl
	end

RestoreStateReg start
	short	m
	plx
	pla
	ply
	pha
	phx
	tya
	sta	>STATEREG
	long	m
	rtl
	end
