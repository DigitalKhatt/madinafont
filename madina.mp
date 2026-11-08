%%  
 % This file is part of DigitalKhatt New Madinah Font which is licensed under the SIL Open Font License, Version 1.1.
 % Go to https://github.com/DigitalKhatt/madinafont/blob/main/LICENSE for full license details.
%%

nametable.familyName := "DigitalKhatt New Madina";
nametable.copyright := "Copyright (c) 2020-2024 Amine Anane, Copyright © 2024 Tarteel Inc.";
lineheight := 88;
penwidth := 88;
joinvector := ljoinvector := rjoinvector := (16,lineheight);
nuqta := 1.4penwidth;
% Enable the historical CoreText workaround for unjustified layout.
boolean isForCoreText; isForCoreText := false;
defAxis("scaleX","SCLX",50,100,200,"params100");
boolean useColoredAya;useColoredAya:=false;