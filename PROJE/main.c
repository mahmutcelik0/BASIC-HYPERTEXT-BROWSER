#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>

/*
DOSYADA PARSE EDILEREK ELEMANLAR OLUSTURULDUKTAN SONRA [-----] OLUSTUGUNDA BUNUN ICERIGI RENK ETIKETI LINK ETIKETI OLABILIR
LINK OLUP OLMADIGINI KONTROL ETMEK ICIN BU METODU KULLANDIM
[----] ICERISINDE | VARSA LINK OLDUGU ICIN TRUE DONDURUR YOKSA FALSE DONDURUR
*/
bool isLink(char *labelArray){
    int number = 0 ;
    while(labelArray[number] !='\0'){
        if(labelArray[number++] == '|') return true;
    }
    return false;

}

/*
DOS ISLETIM SISTEMLERINDE ANSI STANDARTLARI CALISMADIGI ICIN KAYNAKLARDAKI DEFINE LA OLUSTURULAN RENK KODLARI CALISMIYOR.
BUNDAN DOLAYI BOYLE BIR FONKSIYON KULLANDIM ICERISINE SAYI ALIYOR VE SAYININ DEGERINE GORE RENK DEGISIYOR
1 -> BLUE
2 -> GREEN
4 -> RED
7 -> WHITE
SEKLINDE CALISIR
*/
void setColor(int ForgC){

	WORD wColor;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if(GetConsoleScreenBufferInfo(hStdOut,&csbi)){
		wColor = (csbi.wAttributes & 0xF0)+ (ForgC & 0x0F);
		SetConsoleTextAttribute(hStdOut,wColor);
	}

	return;

}

/*
NOT: DOKUMAN ICERIGINDE HERHANGI BIR HATA KONTROLU YAPILMASI ISTENMEDIGI ICIN BAZI KISIMLAR BUG LI OLABILIR. DOGRU CALISAN BIR PROGRAM GELDIGINDE DOGRU CALISIR FAKAT HATA
KONTROLLERI YAPILMADIGI ICIN HATALI BIR PROGRAMDA FARKLI RENKLENDIRMELER YAPABILIR.
NOT: TIKLAYARAK LINKE GECIS YAPMA KISMI FAZLA VAKIT ALACAGI VE YETERLI VAKTIMIN OLMADIGI ICIN PARSE EDILEN DOSYANIN ICINDEKI LINKLER EN SONDA HANGI LINKE GITMEK ISTEDIGINIZ
KONSOLDAN INPUT ALINARAK SORULUYOR VE GIRILEN INPUT A GORE GECIS YAPILIYOR.
*/
/*
ICINE OKUNACAK DOSYANIN POINTER INI ALIR. ILK BASTA MAINDEN GELEN ANA DOSYANIN POINTERINI ALIR VE METODUN EN SONUNDA RECURSIVE YAPI OLUSTURACAK SEKILDE SECILEN LINKTEN
GELEN DOSYANIN POINTER IYLA TEKRAR CAGIRILIR BOYLELIKLE BASKA DOSYAYA GECILMIS OLUR. POINTER I GELEN DOSYANIN PARSE EDILMESI VE KONSOL A UYGUN SEKILLERDE BASTIRILMASI
BURADA GERCEKLESIR
*/
void documentReadingMethod(FILE *documentsPointer){

    char links[20][20]; //OKUNAN HTX DOSYASININ ICINDEKI LINKLERI YANI DOSYA ADLARINI TUTAN IKI BOYUTLU ARRAY. ICERIINDEKI SAYILAR RANDOM
    int linksCount = 0; //OKUNAN HTX DOSASINDA KAC TANE LINK OLDUGUNU TUTAN DEGISKEN

    /*PARSE ISLEMINI GERCEKLESTIREN MAIN WHILE DONGUSU. OKUNAN DOSYANIN SONUNA GELESIYE KADAR CALISMAYA DEVAM EDER VE ICERISINDEKI BLOKLARLA ISLEYIS CALISIR*/
    while(!feof(documentsPointer)){

        /*HTX DOSYASINDA COMMENTLER OLABILECEGI VE BUNLARI IGNORE LAMAMIZ GEREKTIGI ICIN KONTROLLU BIR YAPI OLUSTURDUM
        ILK BASTA KARAKTER ALINIYOR VE WHILE DONGUSUNDE CALISTIKTAN SONRA BU KARAKTER PREVIOUS OLARAK SAKLANIYOR. DONGU TEKRAR CALISIP SONRAKI KARAKTER ALINDIGINDA COMMENT
        DEGILSE PREVIOUS OLAN BASTIRILMIS OLUYOR. ORNEK VERECEK OLURSAM ILK KARAKTER / OLSUN BUNU ALDI PREVIOUS A ATADI SONRASINDAKI KARAKTER * CHARFROMDOCUMENT TE SAKLANDI
        WHILE ICINDEKI IF I SAGLAMADIGI ICIN EKRANA BASTIRILMAMIS OLACAK
        */
        char charFromDocument = fgetc(documentsPointer);
        char previousChar ;


        while(!(charFromDocument == '*' && previousChar == '/' ) && charFromDocument != '[' && !feof(documentsPointer) ){   //DOSYADAKI NORMAL YAZILARIN BASTIRILMASINI SAGLAR

            previousChar = charFromDocument;


            charFromDocument = fgetc(documentsPointer);
            if(!(charFromDocument == '*' && previousChar == '/' )) printf("%c",previousChar);


        }

        //COMMENT GELDIGINDE WHILE DAN CIKILIR VE BU BLOK CALISIR COMMENT KAPATMA */ GELESIYE KADAR KARAKTER ALIR. COMMENT KAPATMA GELMEZSE VE DOSYANIN SONUNA GELINMISSE HATA VERIR
        if(previousChar == '/' && charFromDocument == '*'){
            while(!(charFromDocument== '/' && previousChar=='*') && !feof(documentsPointer)){
                previousChar = charFromDocument ;
                charFromDocument = fgetc(documentsPointer);

            }
            if(feof(documentsPointer)) {
                printf("COMMENTLERI KAPAT");
                exit(0);
            }
        }


        /*DOSYADAN OKUNAN KARAKTER [ ISE BU BLOK CALISIR ICERISINDE LABEL OLUSTURULUR VE NE OLDUGUNA GORE ISLEMLER GERCEKLESIR
        [red] VEYA [blue] ISE RENK DEGISIMI. [..... \ ....htx ] ISE LINK OLUSTURMA TARZI*/
        else if(charFromDocument == '['){
            char labelArray[1000];
            int labelsIndex = 0;

            charFromDocument = fgetc(documentsPointer); // [ SONRASI KARAKTER ALINIR VE ARRAYDE SAKLANIR. ]BULUNASIYA KADAR ARRAY DE SAKLANIR DIGER DURUMLARDAN DOLAYI DONGU DURUYORSA HATA VERECEK

            while(labelsIndex <1000 && !feof(documentsPointer) && charFromDocument != ']'){
                if(isalpha(charFromDocument))
                    charFromDocument = tolower(charFromDocument);

                labelArray[labelsIndex++] = charFromDocument;
                labelArray[labelsIndex] = '\0';




                charFromDocument = fgetc(documentsPointer);

            }


            if(feof(documentsPointer)){ //DOSYANIN SONUNA GELINDIGINDE VE ] ILE LABEL KAPATILMAMISSA HATA VERIR
                printf("LABELLARI KAPAT");
                exit(0);
            }

            else if(labelsIndex >= 1000){
                printf("ETIKET BUYUK");
                exit(0);
            }

            else if(charFromDocument != ']'){   //BASKA BIR YOLLA WHILE DONGUSU DURMUSSA VE ] DEGILSE SON OKUNAN KARAKTER HATA VERIR
                exit(0);
            }


            if(strcmp(labelArray,"red") ==0){   //LABEL ARRAYDE SAKLANAN ETIKET RED ISE RENK DEGISTIRILIR
                setColor(4);
            }

            else if(strcmp(labelArray,"blue") == 0){    //LABEL ARRAYDE SAKLANAN ETIKET BLUE ISE RENK DEGISTIRILIR
                setColor(1);
            }
            else if( (strcmp(labelArray,"end_red") == 0) || (strcmp(labelArray,"end_blue") ==0) ){  //RENKLERIN END I GELMISSE RENK BEYAZA GERI DONER
                setColor(7);
            }


            else if(isLink(labelArray)){    //YUKARDA OLUSTURDUGUM LINK OLUP OLMADIGINI KONTROL EDEN METODTAN TRUE DONERSE CALISACAK FONKSIYON
                setColor(2);    //RENK YESIL YAPILIR

                int numberForLink = 0 ;
                while(labelArray[numberForLink] != '|'){    // [ .... | KISMI YANI SOL KISIM EKRANA BASTIRILIR
                    printf("%c",labelArray[numberForLink++]);

                }


                numberForLink++;    //DOSYADAKI LINK SAYISI 1 ARTTIRILIR EN BASTA OLUSTURULMUSTU
                int linkNumber = 0; //DOSYA ADININ IKI BOYUTLU ARRAY E YAZILMASI
                while(labelArray[numberForLink] != '\0'){   //LABEL ARRAYINDE ELEMANIN SONUNA GELINMEDIGI SURECE CALISACKA DONGU | ....] KISMINI CALISTIRIR


                    char temporary = labelArray[numberForLink];
                    links[linksCount][linkNumber] = temporary;


                    linkNumber++;
                    numberForLink++;

                    links[linksCount][linkNumber] = '\0';   //IKI BOYUTLU ARRAYDE DOSYA ADI ....htx YAZILDIKTAN SONRA \0 EKLENIR STRING OLMASI ICIN
                }
                linksCount++;


                setColor(7);    //RENK BEYAZA GERI DONDURURULUR.
            }


            else{
                printf("THERE IS NOT SUCH A THING LIKE THIS."); //DIGER HATA DURUMLARINDA VEYA ANLAMSIZ KARAKTER DURUMLARINDA CALISACAK ELSE BLOGU
                exit(0);
            }

        }

    }

    /*HER DOSYANIN SONUNDA CALISACAK VE LINK TIKLAMA YERINE OLUSTURDUGUM KISIM. KONSOLDAN GIREBILECEGINIZ INPUTLAR BASILI SEKILDE VE UYGUN SEKILDE INPUT VERDIGINIZDE
    ILGILI DOSYAYA GECIS YAPIYOR
    */
    printf("\n\n--------------------------------------------\n");
    printf("HANGI LINKE GITMEK ISTEDIGINIZI GIRINIZ:  [0 \ %d](CIKMAK ICIN -1)",(linksCount-1));
    int linkToGo ;

    scanf("%d",&linkToGo);

    while((linkToGo < 0 || linkToGo >= linksCount) && linkToGo !=-1) {  //KONSOLDAN ALINAN INPUT LINK SAYISIYLA UYUMLU DEGILSE TEKRAR INPUT ALINIR
        printf("HANGI LINKE GITMEK ISTEDIGINIZI GIRINIZ: [0 \ %d](CIKMAK ICIN -1)",(linksCount-1));
        scanf("%d",&linkToGo);
    }

    if(linkToGo == -1) exit(0); //-1 I PROGRAMI SONLANDIRMA INPUTU OLARAK TANIMLADIM
    FILE *linkPointer;

    /*SECILEN LINKE GITMEYI SAGLAYACAK KISIM. YUKARIDA IKI BOYUTLU ARRAY E YAZILAN VE SONRASINDA SECILEN LINKI UYGUN FORMATTA OLACAK SEKILDE LINKPROCESSING ARRAYINE ATAR
    SONRASINDA ASAGIDA O DOSYA OKUMA ETIKETIYLE ACILIR VE KONSOL TEMIZLENIP BURADAKI GENEL FONKSIYON RECURSIVE SEKILDE CAGIRILIR.*/
    char linkProcessing[20];
    int newLinkLength = 0;
    for(int m = 0 ; links[linkToGo][m] != '\0' ; m++){
        if(isalpha(links[linkToGo][m]) || isdigit(links[linkToGo][m]) || links[linkToGo][m] == '.'|| links[linkToGo][m] == '_'){    //LINKIN OLUSTURULMASI
            linkProcessing[newLinkLength++] = links[linkToGo][m];
            linkProcessing[newLinkLength] = '\0';
        }
    }


    if ((linkPointer = fopen(linkProcessing, "r")) == NULL) {   //DOSYA ACMA ISLEMI
        printf("Dosya acma hatasi!\n");
        exit(1);
    }
    system("@cls||clear");  //KONSOLUN TEMIZLENMESI
    documentReadingMethod(linkPointer); //YENI POINTER ILE CAGIRIM

}


int main()
{
    /*
    PROGRAM ILK CALISTIRILDIGINDA CALISACAK ILK DOSYAYA ERISIM GEREKTIGINDEN MAIN ICINDE DOSYAYI ACTIM VE BUNU DOSYA OKUMA ILE ILGILI ISLEMI YAPAN FONKSIYONA YOLLADIM
    DOSYAYI KAPATTIM
    */
    FILE *mainFilePointer;

    if ((mainFilePointer = fopen("x.htx", "r")) == NULL) {
        printf("Dosya acma hatasi!\n");
        exit(1);
    }

    documentReadingMethod(mainFilePointer);
    fclose(mainFilePointer);

    return 0;
}
