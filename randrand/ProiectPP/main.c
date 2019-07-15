#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct culoare
{
    unsigned char R,G,B;
};
struct imagine
{
    unsigned int w, h, size, padding;
    struct culoare *pixeli_Img;
    unsigned char *header;

};
struct fereastra
{
    double corelatie;
    unsigned int linie,coloana,marime;
    struct culoare culori;
};


unsigned int *xorshift32(unsigned int n, unsigned int seed)
{

/// algoritm care formeaza un vector a
/// cu n elemente pornind de la un seed
/// in care pune numere generate dupa
/// algoritmul xorshift32

    unsigned int *a=NULL, x, i;

    ///aloca dinamic un vector a cu n elemente
    a=(unsigned int *)malloc(n*sizeof(unsigned int));
    if(a==NULL)
        {
            printf("nu s-a putut aloca memorie");
            free(a);
            exit(0);
        }

    ///pune in vectorul a valorile generate
    x=seed;
    for(i=1;i<=n;i++)
    {
        x=x^x<<13;
        x=x^x>>17;
        x=x^x<<5;
        a[i]=x;
    }
    return a;
}

struct imagine incarcaImg(const char *nume_fisier)
{

/// algoritm care salveaza in variabila a de tip
/// 'struct imagine' toate caracteristicile
/// acesteia (inaltime, latime, dimensiune in octeti,
/// pixeli si nr de octeti de padding), dupa ce le-a citit
/// din fisierul imagine transmis ca parametru, si o returneaza

    unsigned int i, j;
    unsigned char pix[3];
    struct imagine a;

    ///deschidem fisierul cu imaginea
    FILE *f=fopen(nume_fisier,"rb");
    if(f==NULL)
        {
            printf("eroare la deschiderea fisierului %s\n", nume_fisier);
            fclose(f);
            exit(0);
        }

    ///alocam memorie pt header si il citim din fiser
    a.header=NULL;
    a.header=(unsigned char *)malloc(54*sizeof(unsigned char));
    if(a.header==NULL)
    {
        printf("nu s-a putut aloca memorie");
        free(a.header);
        exit(0);
    }

    fread(a.header,sizeof(unsigned char),54,f);

    ///citim inaltimea-a.h, latimea-a.w si marimea in octeti-a.size
    fseek(f,2,SEEK_SET);
    fread(&a.size,sizeof(unsigned int),1,f);

    fseek(f,18,SEEK_SET);
    fread(&a.w,sizeof(unsigned int),1,f);
    fread(&a.h,sizeof(unsigned int),1,f);



    ///alocam dinamic memorie pt o matrice liniara a
    a.pixeli_Img=NULL;
    a.pixeli_Img =(struct culoare *) malloc(a.w*a.h*sizeof(struct culoare));
    if(a.pixeli_Img==NULL)
    {
        printf("nu s-a putut aloca memorie");
        free(a.pixeli_Img);
        exit(0);
    }

    ///calculam padding-ul pt linii
    a.padding=4-(a.w*3)%4;
    if(a.padding==4) a.padding=0;

    ///citim imaginea propriu-zisa si punem valorile RGB ale pixelilor in a
    fseek(f,54,SEEK_SET);
    for(i=0;i<a.h;i++)
    {
            for(j=0;j<a.w;j++)
            {
                fread(pix,3*sizeof(unsigned char),1,f);
                a.pixeli_Img[i*a.w+j].B=pix[0];
                a.pixeli_Img[i*a.w+j].G=pix[1];
                a.pixeli_Img[i*a.w+j].R=pix[2];
            }

            ///sarim peste octetii de padding
            fseek(f,a.padding,SEEK_CUR);
    }

    fclose(f);
    return a;
}

void afiseaza_imagine(const char *nume_fisier, struct imagine a)
{

/// algoritm care ia o imagine primita ca parametru
/// stocata in variabila a si o afiseaza in fisierul transmis prin cale

     unsigned int i,j;

     ///cream fisierul in care vom afisa imaginea in forma liniarizata
    FILE *f=fopen(nume_fisier,"wb");
   if(f==NULL)
   {
       printf("eroare la deschiderea fisierului %s\n", nume_fisier);
       fclose(f);
       exit(0);
   }

   ///declaram dinamic un vector padd care va tine octetii de padding
   unsigned char *padd=NULL;
   if(a.padding!=0)
   {
       padd=(unsigned char *)calloc(a.padding,sizeof(unsigned char));
       if(padd==NULL)
       {
           printf("nu s-a putut aloca memorie");
            free(padd);
            exit(0);
       }
   }

   ///afisam header-ul
    fwrite(a.header,sizeof(unsigned char),54,f);

    ///afisam matricea liniarizata a element cu element
 for(i=0;i<a.h;i++)
    {
        for(j=0;j<a.w;j++)
            {
                fwrite(&a.pixeli_Img[i*a.w+j].B,sizeof(unsigned char),1,f);
                fwrite(&a.pixeli_Img[i*a.w+j].G,sizeof(unsigned char),1,f);
                fwrite(&a.pixeli_Img[i*a.w+j].R,sizeof(unsigned char),1,f);
            }
        /// adauga in continuarea sfarsitului de linie
        /// octetii de padding daca acesti exista
        if(a.padding!=0)
            fwrite(padd,sizeof(unsigned char),a.padding,f);
    }

   free(padd);
   fclose(f);
}


void criptare_Img(const char *img_de_criptat, const char *img_criptata, const char *fisier_cheie)
{
    unsigned int i, j, k, r0, r, aux;

    struct imagine a;
    a=incarcaImg(img_de_criptat);

    ///declaram o uniune pt a putea xor-a octetii lui SV si ai nr. din vectorul R cu pixelii imaginii
     union impartireNrInOcteti
    {
        unsigned int nr;
        unsigned char oct[4];
    };

    union impartireNrInOcteti SV;

    ///citim valorile r0 si SV din fisierul cu cheia secreta
    FILE *f=fopen(fisier_cheie,"r");
    fscanf(f,"%u%u",&r0,&SV.nr);
    fclose(f);

    ///initializam pe R cu valorile generate de functia xorshift32
    unsigned int *R=NULL;
    R=xorshift32(a.w*a.h*2-1,r0);

    ///alocam dinamic memorie pt un vector p in care vom pune primele a.h*a.w elemente ale lui R
     unsigned int *p=NULL;
     p=(unsigned int *)malloc(a.w*a.h*sizeof(unsigned int));
     if(p==NULL)
    {
        printf("nu s-a putut aloca memorie");
        free(p);
        exit(0);
    }

   ///initializam p
    for(i=0;i<a.w*a.h;i++)
        p[i]=i;

    ///generam permutarea aleatoare p dupa algoritmul Durstenfeld
    ///folosing numerele generate cu xorshift32 care au fost puse in R

    for(k=a.h*a.w-1;k>=1;k--)
    {
        r=R[a.h*a.w-k]%(k+1);
        aux=p[r];
        p[r]=p[k];
        p[k]=aux;
    }

    ///intoarcem liniile imaginii pt a incepe de la ultima
     unsigned char auxi;
    for(i=0;i<a.h/2;i++)
        for(j=0;j<a.w;j++)
        {
            auxi=a.pixeli_Img[i*a.w+j].R;
            a.pixeli_Img[i*a.w+j].R=a.pixeli_Img[(a.h-1-i)*a.w+j].R;
            a.pixeli_Img[(a.h-1-i)*a.w+j].R=auxi;

            auxi=a.pixeli_Img[i*a.w+j].G;
            a.pixeli_Img[i*a.w+j].G=a.pixeli_Img[(a.h-1-i)*a.w+j].G;
            a.pixeli_Img[(a.h-1-i)*a.w+j].G=auxi;

            auxi=a.pixeli_Img[i*a.w+j].B;
            a.pixeli_Img[i*a.w+j].B=a.pixeli_Img[(a.h-1-i)*a.w+j].B;
            a.pixeli_Img[(a.h-1-i)*a.w+j].B=auxi;
        }


    ///declaram struct imagine a_cript in care vom pastra imaginea criptata in forma liniarizata
    struct imagine a_cript;
    a_cript=incarcaImg(img_de_criptat);

    ///formam vectorul a_cript dupa regula permutarii p
    for(i=0;i<a.h*a.w;i++)
    {
        a_cript.pixeli_Img[p[i]].R=a.pixeli_Img[i].R;
        a_cript.pixeli_Img[p[i]].G=a.pixeli_Img[i].G;
        a_cript.pixeli_Img[p[i]].B=a.pixeli_Img[i].B;
    }

    ///xor-am pixelii din imagine dupa regula data
    for(k=0;k<a.h*a.w;k++)
    {
        union impartireNrInOcteti x;
        x.nr=R[a.h*a.w+k];
        if(k==0)
           {
            a_cript.pixeli_Img[k].R=SV.oct[2]^a_cript.pixeli_Img[k].R^x.oct[2];
            a_cript.pixeli_Img[k].G=SV.oct[1]^a_cript.pixeli_Img[k].G^x.oct[1];
            a_cript.pixeli_Img[k].B=SV.oct[0]^a_cript.pixeli_Img[k].B^x.oct[0];
           }
        else
        {
            a_cript.pixeli_Img[k].R=a_cript.pixeli_Img[k-1].R^a_cript.pixeli_Img[k].R^x.oct[2];
            a_cript.pixeli_Img[k].G=a_cript.pixeli_Img[k-1].G^a_cript.pixeli_Img[k].G^x.oct[1];
            a_cript.pixeli_Img[k].B=a_cript.pixeli_Img[k-1].B^a_cript.pixeli_Img[k].B^x.oct[0];
        }
    }

    ///intoarcem din nou liniile pt a readuce imaginea la normal
    unsigned char auzi;
    for(i=0;i<a_cript.h/2;i++)
        for(j=0;j<a_cript.w;j++)
        {
            auzi=a_cript.pixeli_Img[i*a_cript.w+j].R;
            a_cript.pixeli_Img[i*a_cript.w+j].R=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].R;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].R=auzi;

            auzi=a_cript.pixeli_Img[i*a_cript.w+j].G;
            a_cript.pixeli_Img[i*a_cript.w+j].G=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].G;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].G=auzi;

            auzi=a_cript.pixeli_Img[i*a_cript.w+j].B;
            a_cript.pixeli_Img[i*a_cript.w+j].B=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].B;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].B=auzi;
        }

    ///salvam imaginea criptata in memorie
    afiseaza_imagine(img_criptata,a_cript);

    free(R);
    free(p);
    free(a.pixeli_Img);
    free(a.header);
    free(a_cript.pixeli_Img);
    free(a_cript.header);
}


void decriptare_Img(const char *img_criptata,const char *img_decriptata,const char *fisier_cheie)
{
    unsigned int i, j, k, r0, r, aux;

     union impartireNrInOcteti
    {
        unsigned int nr;
        unsigned char oct[4];
    };

    union impartireNrInOcteti SV;

    FILE *f=fopen(fisier_cheie,"r");
    fscanf(f,"%u%u",&r0,&SV.nr);
    fclose(f);

    ///punem ina_cript imaginea criptata sub forma liniarizata
    struct imagine a_cript;
    a_cript=incarcaImg(img_criptata);

    unsigned int *R;
    R=xorshift32(a_cript.w*a_cript.h*2-1,r0);

     unsigned int *p=NULL;
     p=(unsigned int *)malloc((a_cript.w*a_cript.h)*sizeof(unsigned int));
     if(p==NULL)
    {
        printf("nu s-a putut aloca memorie");
        free(p);
        exit(0);
    }

    for(i=0;i<a_cript.w*a_cript.h;i++)
        p[i]=i;

    for(k=a_cript.h*a_cript.w-1;k>=1;k--)
    {
        r=R[a_cript.h*a_cript.w-k]%(k+1);
        aux=p[r];
        p[r]=p[k];
        p[k]=aux;
    }

    ///calculam inversa permutarii p
    unsigned int *pinv=NULL;
    pinv=(unsigned int *)malloc(a_cript.h*a_cript.w*sizeof(unsigned int));

    for(k=0;k<a_cript.h*a_cript.w;k++)
        pinv[p[k]]=k;

    ///declaram un vector dinamic intermediar in care vom face inversele operatiilor de xor-are
    struct imagine a_intermediara;
    a_intermediara=incarcaImg(img_criptata);

    ///intoarcem liniile pt a incepe de la ultima
    unsigned char auxi;
    for(i=0;i<a_cript.h/2;i++)
        for(j=0;j<a_cript.w;j++)
        {
            auxi=a_cript.pixeli_Img[i*a_cript.w+j].R;
            a_cript.pixeli_Img[i*a_cript.w+j].R=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].R;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].R=auxi;

            auxi=a_cript.pixeli_Img[i*a_cript.w+j].G;
            a_cript.pixeli_Img[i*a_cript.w+j].G=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].G;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].G=auxi;

            auxi=a_cript.pixeli_Img[i*a_cript.w+j].B;
            a_cript.pixeli_Img[i*a_cript.w+j].B=a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].B;
            a_cript.pixeli_Img[(a_cript.h-1-i)*a_cript.w+j].B=auxi;
        }

    ///xor-am pixelii din imagine dupa regula data
    for(k=0;k<a_cript.h*a_cript.w;k++)
    {
        union impartireNrInOcteti x;
        x.nr=R[a_cript.h*a_cript.w+k];

        if(k==0)
           {
            a_intermediara.pixeli_Img[k].R=SV.oct[2]^a_cript.pixeli_Img[k].R^x.oct[2];
            a_intermediara.pixeli_Img[k].G=SV.oct[1]^a_cript.pixeli_Img[k].G^x.oct[1];
            a_intermediara.pixeli_Img[k].B=SV.oct[0]^a_cript.pixeli_Img[k].B^x.oct[0];
           }
        else
        {
            a_intermediara.pixeli_Img[k].R=a_cript.pixeli_Img[k-1].R^a_cript.pixeli_Img[k].R^x.oct[2];
            a_intermediara.pixeli_Img[k].G=a_cript.pixeli_Img[k-1].G^a_cript.pixeli_Img[k].G^x.oct[1];
            a_intermediara.pixeli_Img[k].B=a_cript.pixeli_Img[k-1].B^a_cript.pixeli_Img[k].B^x.oct[0];        }
    }


    ///repozitionam pixelii din vectorul intermediar in unul final, a_decript, dupa pozitiile din permutarea inversa
    struct imagine a_decript;
    a_decript=incarcaImg(img_criptata);

    for(i=0;i<a_intermediara.h*a_intermediara.w;i++)
    {
        a_decript.pixeli_Img[pinv[i]].R=a_intermediara.pixeli_Img[i].R;
        a_decript.pixeli_Img[pinv[i]].G=a_intermediara.pixeli_Img[i].G;
        a_decript.pixeli_Img[pinv[i]].B=a_intermediara.pixeli_Img[i].B;
    }

    ///intoarcem liniile pt a readuce imaginea la normal
     unsigned char auzi;
    for(i=0;i<a_decript.h/2;i++)
        for(j=0;j<a_decript.w;j++)
        {
            auzi=a_decript.pixeli_Img[i*a_decript.w+j].R;
            a_decript.pixeli_Img[i*a_decript.w+j].R=a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].R;
            a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].R=auzi;

            auzi=a_decript.pixeli_Img[i*a_decript.w+j].G;
            a_decript.pixeli_Img[i*a_decript.w+j].G=a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].G;
            a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].G=auzi;

            auzi=a_decript.pixeli_Img[i*a_decript.w+j].B;
            a_decript.pixeli_Img[i*a_decript.w+j].B=a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].B;
            a_decript.pixeli_Img[(a_decript.h-1-i)*a_decript.w+j].B=auzi;
        }

    ///salvam imaginea decriptata in memorie
    afiseaza_imagine(img_decriptata,a_decript);

    free(R);
    free(p);
    free(pinv);
    free(a_intermediara.pixeli_Img);
    free(a_intermediara.header);
    free(a_cript.pixeli_Img);
    free(a_cript.header);
    free(a_decript.pixeli_Img);
    free(a_decript.header);
}

void test_chi(char *imagine)
{
    struct imagine a;
    a=incarcaImg(imagine);

    unsigned int i, j, fi;
    float fbarat=(a.w*a.h)/256, x2;

    ///testul chi pt canalul rosu de culoare
    x2=0;
    for(i=0;i<256;i++)
        {
            fi=0;
            for(j=0;j<a.w*a.h;j++)
                if(a.pixeli_Img[j].R==i) fi++;
            x2=x2+(fi-fbarat)*(float)(fi-fbarat)/fbarat;
        }
    printf("R: %0.3f",x2);

    ///testul chi pt canalul verde de culoare
    x2=0;
    for(i=0;i<256;i++)
        {
            fi=0;
            for(j=0;j<a.w*a.h;j++)
                if(a.pixeli_Img[j].G==i) fi++;
            x2=x2+(fi-fbarat)*(float)(fi-fbarat)/fbarat;
        }
    printf("\nG: %0.3f",x2);

    ///testul chi pt canalul albastru de culoare
    x2=0;
    for(i=0;i<256;i++)
        {
            fi=0;
            for(j=0;j<a.w*a.h;j++)
                if(a.pixeli_Img[j].B==i) fi++;
            x2=x2+(fi-fbarat)*(float)(fi-fbarat)/fbarat;
        }
    printf("\nB: %0.3f",x2);

    free(a.header);
    free(a.pixeli_Img);
}



void grayscale(const char *imagineColor, const char *imagineGrayscale)
{
    unsigned int i,j;
    unsigned char aux;

    struct imagine a;
    a=incarcaImg(imagineColor);

    for(i=0;i<a.h;i++)
    {
        for(j=0;j<a.w;j++)
        {
            aux = 0.299*a.pixeli_Img[i*a.w+j].R + 0.587*a.pixeli_Img[i*a.w+j].G + 0.114*a.pixeli_Img[i*a.w+j].B;
			a.pixeli_Img[i*a.w+j].R=a.pixeli_Img[i*a.w+j].G=a.pixeli_Img[i*a.w+j].B = aux;
        }
    }
    afiseaza_imagine(imagineGrayscale,a);

    free(a.header);
    free(a.pixeli_Img);
}

void coloreaza(const char *img, struct fereastra f1, struct culoare C, struct imagine *a, struct imagine s)
{
    unsigned int i,j;

    for(i=f1.linie;i<s.h+f1.linie;i++)
        {
            (*a).pixeli_Img[i*((*a).w)+f1.coloana].R=C.R;
            (*a).pixeli_Img[i*((*a).w)+f1.coloana].G=C.G;
            (*a).pixeli_Img[i*((*a).w)+f1.coloana].B=C.B;
            (*a).pixeli_Img[i*((*a).w)+f1.coloana+s.w-1].R=C.R;
            (*a).pixeli_Img[i*((*a).w)+f1.coloana+s.w-1].G=C.G;
            (*a).pixeli_Img[i*((*a).w)+f1.coloana+s.w-1].B=C.B;
        }
    for(j=f1.coloana;j<s.w+f1.coloana;j++)
        {
            (*a).pixeli_Img[f1.linie*((*a).w)+j].R=C.R;
            (*a).pixeli_Img[f1.linie*((*a).w)+j].G=C.G;
            (*a).pixeli_Img[f1.linie*((*a).w)+j].B=C.B;
            (*a).pixeli_Img[(f1.linie+s.h-1)*((*a).w)+j].R=C.R;
            (*a).pixeli_Img[(f1.linie+s.h-1)*((*a).w)+j].G=C.G;
            (*a).pixeli_Img[(f1.linie+s.h-1)*((*a).w)+j].B=C.B;
        }

}


struct fereastra *template_matching(const char *img, const char *sablon, double prag, struct imagine a)
{
    unsigned int j, l, c, i, n=0;
    double Vs=0, Vf=0, S=0, fiBarat=0, corr=0;

    struct fereastra *vectorFI=NULL;
    struct imagine s;

    grayscale(sablon,"cifraGray.bmp");
    s=incarcaImg("cifraGray.bmp");

    ///stergem fisierul cifraGray.bmp, nemaiavand nevoie de el
    if (remove("cifraGray.bmp")!=0)
      printf("fisierul cifraGray.bmp nu s-a putut sterge");

    ///calculez S barat
    S=0;
    for(l=0;l<s.h*s.w;l++)
            S=S+s.pixeli_Img[l].R;

    S=(double)S/(s.h*s.w);

    ///calculez sigma S
    Vs=0;
    for(l=0;l<s.h*s.w;l++)
        {
            Vs=Vs+(double)(s.pixeli_Img[l].R-S)*(double)(s.pixeli_Img[l].R-S);
        }
    Vs=(double)Vs/(s.h*s.w-1);
    Vs=(double)sqrt(Vs);

    for(i=0;i<a.h-s.h;i++)
        for(j=0;j<a.w-s.w;j++)
            {
                ///calculez fI barat

                fiBarat=0;
                for(l=i;l<s.h+i;l++)
                    for(c=j;c<s.w+j;c++)
                        {
                            fiBarat=fiBarat+a.pixeli_Img[l*a.w+c].R;
                        }

                fiBarat=(double)fiBarat/(s.h*s.w);

                ///calculez sigma F
                Vf=0;
                for(l=i;l<s.h+i;l++)
                    for(c=j;c<s.w+j;c++)
                        {
                            Vf=Vf+(double)(a.pixeli_Img[l*a.w+c].R-fiBarat)*(a.pixeli_Img[l*a.w+c].R-fiBarat);
                        }
                Vf=(double)Vf/(s.h*s.w-1);
                Vf=(double)sqrt(Vf);

                ///calculez corelatia
                corr=0;
                 for(l=i;l<s.h+i;l++)
                    for(c=j;c<s.w+j;c++)
                    {
                        corr=corr+(double)(a.pixeli_Img[l*a.w+c].R-fiBarat)*(s.pixeli_Img[(l-i)*s.w+c-j].R-S);
                    }

                corr=(double)corr/Vf;
                corr=(double)corr/Vs;
                corr=(double)corr/(s.h*s.w);


                if(corr>prag)
                    {
                        if(vectorFI!=NULL)
                                vectorFI=(struct fereastra *)realloc(vectorFI, sizeof(struct fereastra)*(n+1));
                        else vectorFI=(struct fereastra *)malloc(sizeof(struct fereastra));

                        if(vectorFI==NULL)
                        {
                            printf("nu s-a putut aloca memorie");
                            free(vectorFI);
                            exit(0);
                        }
                        vectorFI[n].linie=i;
                        vectorFI[n].coloana=j;
                        vectorFI[n].corelatie=corr;
                        n++;
                    }
            }
    for(i=0;i<n;i++)
        vectorFI[i].marime=0;

    vectorFI[0].marime=n;


    free(s.header);
    free(s.pixeli_Img);

    return vectorFI;

}
int cmp(const void *a, const void *b)
{
    struct fereastra vect1, vect2;
    vect1=*(struct fereastra *)a;
    vect2=*(struct fereastra *)b;
    if(vect1.corelatie<vect2.corelatie) return 1;
    if(vect1.corelatie>vect2.corelatie) return -1;
    return 0;
}
void sortare(struct fereastra **vectorFI)
{
    unsigned int n=(*vectorFI)[0].marime;
    qsort(*vectorFI,n,sizeof(struct fereastra),cmp);
    (*vectorFI)[0].marime=n;
}

double suprapunere(struct fereastra di, struct fereastra dj,struct imagine s)
{
    unsigned int intersect=0, reuniune=0;
    double supp=0;

    if(dj.linie<di.linie+s.h && dj.linie>=di.linie && dj.coloana<di.coloana+s.w && dj.coloana>=di.coloana) intersect=(di.coloana+s.w-dj.coloana)*(di.linie+s.h-dj.linie);
        else if(dj.linie<di.linie+s.h && dj.linie>=di.linie && dj.coloana+s.w<=di.coloana+s.w && dj.coloana+s.w>di.coloana) intersect=(dj.coloana+s.w-di.coloana)*(di.linie+s.h-dj.linie);
                else if(di.linie<dj.linie+s.h && di.linie>dj.linie && di.coloana<dj.coloana+s.w && di.coloana>=dj.coloana) intersect=(dj.coloana+s.w-di.coloana)*(dj.linie+s.h-di.linie);
                        else if(di.linie<dj.linie+s.h && di.linie>dj.linie && di.coloana+s.w<=dj.coloana+s.w && di.coloana+s.w>dj.coloana) intersect=(di.coloana+s.w-dj.coloana)*(dj.linie+s.h-di.linie);

     reuniune=2*s.h*s.w-intersect;

     supp=(double)intersect/reuniune;
     return supp;
}

void elim_nonMaxime(struct fereastra **D, struct imagine s)
{
    unsigned int i,j,k,n;
    n=(*D)[0].marime;

    sortare(D);
    i=0;
    while(i<n-1)
    {
        j=i+1;
        while(j<n)
        {
            if(suprapunere((*D)[i],(*D)[j],s)>0.2)
            {
                k=j;
                while(k<n-1)
                    {
                        (*D)[k]=(*D)[k+1];
                        k++;
                    }
                n--;
            }
            else j++;
        }
    i++;
    }
    (*D)[0].marime=n;
}
int main()
{

    printf("---------PARTEA DE CRIPTARE---------");


    char *sir1=NULL, *sir2=NULL, *sir3=NULL;

    sir1=(char *)malloc(50*sizeof(char));
    sir2=(char *)malloc(50*sizeof(char));
    sir3=(char *)malloc(50*sizeof(char));

    ///citim numele imaginii de criptat, numele imaginii in care punem imaginea criptata si cheia secreta
    printf("\n\n");
    printf("imaginea pe care o criptam: ");
    fgets(sir1,50,stdin);
    printf("numele pentru imaginea criptata: ");
    fgets(sir2,50,stdin);
    printf("fisierul cu cheia secreta: ");
    fgets(sir3,50,stdin);

    sir1[strlen(sir1) - 1] = '\0';
    sir2[strlen(sir2) - 1] = '\0';
    sir3[strlen(sir3) - 1] = '\0';

    criptare_Img(sir1,sir2,sir3);

    printf("\n\n");

    char *sir4=NULL, *sir5=NULL, *sir6=NULL;

    sir4=(char *)malloc(50*sizeof(char));
    sir5=(char *)malloc(50*sizeof(char));
    sir6=(char *)malloc(50*sizeof(char));

    ///citim numele imaginii de decriptat, numele imaginii in care punem imaginea decriptata si cheia secreta
    printf("imaginea pe care o decriptam: ");
    fgets(sir4,50,stdin);
    printf("numele pentru imaginea decriptata: ");
    fgets(sir5,50,stdin);
    printf("fisierul cu cheia secreta: ");
    fgets(sir6,50,stdin);

    sir4[strlen(sir4) - 1] = '\0';
    sir5[strlen(sir5) - 1] = '\0';
    sir6[strlen(sir6) - 1] = '\0';

    decriptare_Img(sir4,sir5,sir6);

    ///valorile testului chi pt imaginea initiala si cea criptata
    printf("\n\n");
    printf("testul chi^2 pentru imaginea initiala:\n");
    test_chi(sir1);
    printf("\n\ntestul chi^2 pentru imaginea criptata:\n");
    test_chi(sir2);

    printf("\n\n");

    free(sir1);
    free(sir2);
    free(sir3);
    free(sir4);
    free(sir5);
    free(sir6);



    printf("---------PARTEA DE TEMPLATE MATCHING---------");


    printf("\n\n");
    unsigned int i,n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n;


    char *imagineDeTestat=NULL, *sablon0=NULL, *sablon1=NULL, *sablon2=NULL, *sablon3=NULL, *sablon4=NULL;
    char *sablon5=NULL, *sablon6=NULL, *sablon7=NULL, *sablon8=NULL, *sablon9=NULL;

    imagineDeTestat=(char *)malloc(50*sizeof(char));
    sablon0=(char *)malloc(50*sizeof(char));
    sablon1=(char *)malloc(50*sizeof(char));
    sablon2=(char *)malloc(50*sizeof(char));
    sablon3=(char *)malloc(50*sizeof(char));
    sablon4=(char *)malloc(50*sizeof(char));
    sablon5=(char *)malloc(50*sizeof(char));
    sablon6=(char *)malloc(50*sizeof(char));
    sablon7=(char *)malloc(50*sizeof(char));
    sablon8=(char *)malloc(50*sizeof(char));
    sablon9=(char *)malloc(50*sizeof(char));


    ///citim numele imaginii, creeam versiunea sa grayscale si o liniarizam

    printf("numele imagini pe care vom aplica algoritmul de template matching: ");
    fgets(imagineDeTestat,50,stdin);
    imagineDeTestat[strlen(imagineDeTestat) - 1] = '\0';

    struct imagine a;
    grayscale(imagineDeTestat,"imagineDeTestatGray.bmp");
    a=incarcaImg("imagineDeTestatGray.bmp");

    ///stergem fisierul imagineDeTestatGray, nemaiavand nevoie de el
    if (remove("imagineDeTestatGray.bmp")!=0)
      printf("fisierul imagineDeTestatGray.bmp nu s-a putut sterge");

    ///citim numele sabloanelor

    printf("numele sablonului 0: ");
    fgets(sablon0,50,stdin);
    printf("numele sablonului 1: ");
    fgets(sablon1,50,stdin);
    printf("numele sablonului 2: ");
    fgets(sablon2,50,stdin);
    printf("numele sablonului 3: ");
    fgets(sablon3,50,stdin);
    printf("numele sablonului 4: ");
    fgets(sablon4,50,stdin);
    printf("numele sablonului 5: ");
    fgets(sablon5,50,stdin);
    printf("numele sablonului 6: ");
    fgets(sablon6,50,stdin);
    printf("numele sablonului 7: ");
    fgets(sablon7,50,stdin);
    printf("numele sablonului 8: ");
    fgets(sablon8,50,stdin);
    printf("numele sablonului 9: ");
    fgets(sablon9,50,stdin);

    sablon0[strlen(sablon0) - 1] = '\0';
    sablon1[strlen(sablon1) - 1] = '\0';
    sablon2[strlen(sablon2) - 1] = '\0';
    sablon3[strlen(sablon3) - 1] = '\0';
    sablon4[strlen(sablon4) - 1] = '\0';
    sablon5[strlen(sablon5) - 1] = '\0';
    sablon6[strlen(sablon6) - 1] = '\0';
    sablon7[strlen(sablon7) - 1] = '\0';
    sablon8[strlen(sablon8) - 1] = '\0';
    sablon9[strlen(sablon9) - 1] = '\0';

    struct fereastra *f0=NULL,*f1=NULL,*f2=NULL,*f3=NULL,*f4=NULL;
    struct fereastra *f5=NULL,*f6=NULL,*f7=NULL,*f8=NULL,*f9=NULL;


    ///salvam ferestrele cu corelatia mai mare decat pragul intr-un vector pt fiecare sablon
    f0=template_matching(imagineDeTestat,sablon0,0.5,a);
    n0=f0[0].marime;
    f1=template_matching(imagineDeTestat,sablon1,0.5,a);
    n1=f1[0].marime;
    f2=template_matching(imagineDeTestat,sablon2,0.5,a);
    n2=f2[0].marime;
    f3=template_matching(imagineDeTestat,sablon3,0.5,a);
    n3=f3[0].marime;
    f4=template_matching(imagineDeTestat,sablon4,0.5,a);
    n4=f4[0].marime;
    f5=template_matching(imagineDeTestat,sablon5,0.5,a);
    n5=f5[0].marime;
    f6=template_matching(imagineDeTestat,sablon6,0.5,a);
    n6=f6[0].marime;
    f7=template_matching(imagineDeTestat,sablon7,0.5,a);
    n7=f7[0].marime;
    f8=template_matching(imagineDeTestat,sablon8,0.5,a);
    n8=f8[0].marime;
    f9=template_matching(imagineDeTestat,sablon9,0.5,a);
    n9=f9[0].marime;


    ///construim vectorul D cu toate detectiile din toate sabloanele
    /// si alegem culorile pt incadratul detectiilor

    struct fereastra *D=NULL;
    n=n0+n1+n2+n3+n4+n5+n6+n7+n8+n9;
    D=(struct fereastra *)malloc(n*sizeof(struct fereastra));
    if(D==NULL)
    {
        printf("nu s-a putut aloca memorie");
        free(D);
        exit(0);
    }
    n=0;

    for(i=0;i<n0;i++)
    {
        f0[i].culori.R=255; f0[i].culori.G=0; f0[i].culori.B=0;    ///rosu
        D[n++]=f0[i];
    }
    for(i=0;i<n1;i++)
    {
        f1[i].culori.R=255; f1[i].culori.G=255; f1[i].culori.B=0;  ///galben
        D[n++]=f1[i];
    }
     for(i=0;i<n2;i++)
    {
        f2[i].culori.R=0; f2[i].culori.G=255; f2[i].culori.B=0;    ///verde
        D[n++]=f2[i];
    }
     for(i=0;i<n3;i++)
     {
        f3[i].culori.R=0; f3[i].culori.G=255; f3[i].culori.B=255; ///cyan
        D[n++]=f3[i];
     }
     for(i=0;i<n4;i++)
     {
        f4[i].culori.R=255; f4[i].culori.G=0; f4[i].culori.B=255;  ///magenta
        D[n++]=f4[i];
     }
     for(i=0;i<n5;i++)
      {
        f5[i].culori.R=0; f5[i].culori.G=0; f5[i].culori.B=255;    ///albastru
        D[n++]=f5[i];
      }
     for(i=0;i<n6;i++)
    {
        f6[i].culori.R=192; f6[i].culori.G=192; f6[i].culori.B=192; ///argintiu
        D[n++]=f6[i];
    }
     for(i=0;i<n7;i++)
     {
        f7[i].culori.R=255; f7[i].culori.G=140; f7[i].culori.B=0;  ///albastru
        D[n++]=f7[i];
     }
     for(i=0;i<n8;i++)
     {
        f8[i].culori.R=128; f8[i].culori.G=0; f8[i].culori.B=128;///magenta
        D[n++]=f8[i];
     }
     for(i=0;i<n9;i++)
     {
        f9[i].culori.R=128; f9[i].culori.G=0; f9[i].culori.B=0;  ///albastru
        D[n++]=f9[i];
     }

    for(i=1;i<n;i++)
        D[i].marime=0;
     D[0].marime=n;


    struct imagine img,s;
    img=incarcaImg(imagineDeTestat);
    s=incarcaImg(sablon0);

    ///sortam si eliminam non_maximele
    elim_nonMaxime(&D,s);


    ///conturam ferestrele si salvam imaginea colorata in structura img
    for(i=0;i<D[0].marime;i++)
        coloreaza(imagineDeTestat,D[i],D[i].culori,&img,s);

    ///afisam imaginea liniarizata colorata img in fisierul imagineColorata.bmp
     afiseaza_imagine("imagineColorata.bmp",img);

    printf("\nimaginea %s colorata se afla in fisierul imagineColorata.bmp\n",imagineDeTestat);


    free(D);

    free(a.header);
    free(a.pixeli_Img);

    free(s.header);
    free(s.pixeli_Img);

    free(img.header);
    free(img.pixeli_Img);

    free(imagineDeTestat);
    free(sablon0);
    free(sablon1);
    free(sablon2);
    free(sablon3);
    free(sablon4);
    free(sablon5);
    free(sablon6);
    free(sablon7);
    free(sablon8);
    free(sablon9);

    free(f0);
    free(f1);
    free(f2);
    free(f3);
    free(f4);
    free(f5);
    free(f6);
    free(f7);
    free(f8);
    free(f9);

    return 0;
}
