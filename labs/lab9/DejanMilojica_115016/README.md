## ZADATAK
	Kreiranje tri programske niti, koje odlikuje problem sa inverzijom prioriteta. Sve tri niti, posjeduju razlicite prioritete(NAJMANJI, SREDNJI i NAJVECI), na kojim se zasniva prednost prilikom izvrsavanja(ukoliko imamo vise niti razlicitih prioriteta, koje su spremne za izvrsavanje, prednost od strane schedulera ima ona nit sa najvisim prioritetom, sto se u ovom slucaju reprezentuje kao najveci broj.) Kako bi se izazvala inverzija prioriteta, prilikom kreiranja, odnosno pokretanja, pokrece se nit sa najmanjim prioritetom, te ulazi u zasticeni dio, koji dijeli sa niti najviseg prioriteta. Kada nit najviseg prioriteta postane spremna za izvrsavanje, dobija priliku od strane schedulera, medjutim, kako je prva nit zakljucala dio zajednickog programskog koda, ona se blokira, dok se taj dio ne oslobodi. U medjuvremenu, aktivira se i srednja nit, koja ima srednji prioritet, i usljed pojave da je veci od prioriteta prve niti, dobija priliku za izvrsavanje, te se izvrsava. Izlaskom prve niti najmanjeg prioriteta iz zasticene zone, scheduler daje prednost niti najviseg prioriteta. Ostatak izvrsavanja, se obavlja standardno, jer imamo dvije niti razlicitih prioriteta.

### Kompajliranje + Prebacivanje na RPI:
	Kroskompajliranje, kao i prebacivanje na zeljenu platformu, vrsi se pomocu skripte "skripta.sh". Unutar nje, definisan je port za prebacivanje, zeljena putanja, direktorijum i slicno.

### Pokretanje:
	Pokretanje se vrsi sa "sudo ./naziv_izvrsnog_objekta"
