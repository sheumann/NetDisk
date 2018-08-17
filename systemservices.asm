* Wrappers for GS/OS system service calls

* These are designed to be called from within GS/OS, e.g. in a driver.

	case	on

* System service routines
CACHE_FIND_BLK gequ $01FC04
CACHE_ADD_BLK  gequ $01FC08
SWAP_OUT       gequ $01FC34
SET_SYS_SPEED  gequ $01FC50
MOVE_INFO      gequ $01FC70
SIGNAL         gequ $01FC88
SET_DISKSW     gequ $01FC90
SUP_DRVR_DISP  gequ $01FCA4
INSTALL_DRIVER gequ $01FCA8

* GS/OS direct page locations
deviceNum gequ $00
callNum   gequ $02

* Driver call number
Driver_Read gequ $0002

dummy	private
	end

CacheFindBlk start
	phb
	phd
	lda	>gsosDP		; set DP to GS/OS direct page
	tcd
	
	jsl	ForceLCBank1	; force in language card bank 1
	pha			; save old state reg
	
	clc			; search by device number
	jsl	CACHE_FIND_BLK	; call the system service
	php			; save result (carry flag)
	php
	pld
	
	jsl	RestoreStateReg	; restore old state reg
	
	tdc			; get result
	and	#$0001		; get just carry flag value as result
	eor	#$0001		; invert sense, so TRUE=block was found
	pld			; restore direct page
	plb
	rtl
	end


CacheAddBlk start
	phb
	phd
	lda	>gsosDP		; set DP to GS/OS direct page
	tcd
	
	jsl	ForceLCBank1	; force in language card bank 1
	pha			; save old state reg
	
	jsl	CACHE_ADD_BLK	; call the system service
	php			; save result (carry flag)
	php
	pld
	
	jsl	RestoreStateReg	; restore old state reg
	
	tdc			; get result
	and	#$0001		; get just carry flag value as result
	eor	#$0001		; invert sense, so TRUE=block was added
	pld			; restore direct page
	plb
	rtl
	end


SetDiskSw start
	phd
	lda	>gsosDP		; set DP to GS/OS direct page
	tcd
	
	jsl	ForceLCBank1	; force in language card bank 1
	pha			; save old state reg
	
	pei	deviceNum	; Save deviceNum & CallNum
	pei	callNum
	lda	Driver_Read	; and adjust callNum
	sta	callNum		; per GS/OS TN #7
	
	jsl	SET_DISKSW	; call the system service

	pla			; restore callNum & deviceNum
	sta	callNum
	pla
	sta	deviceNum
	
	jsl	RestoreStateReg	; restore old state reg
	
	pld			; restore direct page
	rtl
	end
