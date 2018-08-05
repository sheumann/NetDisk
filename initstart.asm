	case	on

dummy	private
	jmp	InitStart
	end

InitStart private
	tay
	tsc
	clc
	adc	#4
	sta	>unloadFlagPtr
	lda	#0
	sta	>unloadFlagPtr+2
	tya
	end
