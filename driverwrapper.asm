	case	on

dummy	private
	end

DriverWrapper start
	pea	$0000		; direct page pointer
	phd
	
	pha			; call number
	
	jsl	DriverDispatch
	
	tay			; set carry based on return value
	bne	err_ret
	clc
	rtl
err_ret	sec
	rtl
	end
