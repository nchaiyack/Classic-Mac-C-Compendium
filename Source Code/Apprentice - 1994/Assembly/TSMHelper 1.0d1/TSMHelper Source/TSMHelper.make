TSMHelper �� TSMHelper.a.o
	link TSMHelper.a.o -rt INIT=11 -o TSMHelper -t 'INIT' -c 'TSM+' �
		-ra =16 -sn "Main=TSMHelper" 
	setfile TSMHelper -a B

TSMHelper �� TSMHelper.r
	rez	TSMHelper.r -append -o TSMHelper -t 'INIT' -c 'TSM+'
	setfile TSMHelper -a B
