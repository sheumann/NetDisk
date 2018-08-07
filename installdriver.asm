	case	on

GSOSBusy gequ	$E100BE

INSTALL_DRIVER gequ $01FCA8

dummy	private
	end

* Adapted from procedure shown in Brutal Deluxe GS/OS Internals book
InstallDriver start
	phb			; set data bank to $E1 (to access GSOSBusy)
	pea	$E1E1
	plb
	plb
	
	lda	#$8000		; check and set GS/OS busy flag
	tsb	|GSOSBusy
	bne	exit		; bail out if GS/OS is busy
	
	phd
	lda	>gsosDP		; set DP to GS/OS direct page
	tcd			;  (should be unnecessary for INSTALL_DRIVER)
	
	jsl	ForceLCBank1	; force in language card bank 1
	pha			; save old state reg
	
	ldx	#dibList
	ldy	#^dibList
	jsl	INSTALL_DRIVER	; install the driver
	tcd			; save INSTALL_DRIVER result
	
	jsl	RestoreStateReg	; restore old state reg
	
	lda	#$8000		; clear GS/OS busy flag
	trb	|GSOSBusy
	
	tdc			; return INSTALL_DRIVER result
	pld			; restore direct page
	
exit	plb
	rtl
	end
