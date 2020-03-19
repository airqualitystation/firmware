#include <stdio.h>

#define CHAR_MAX_LINE 500
FILE *in; 
FILE *out; 

char line[CHAR_MAX_LINE];

int idx =0; 

char c; 

char year[5];
char month[3];
char day[3];

char h[3];
char m[3];
char s[3]; 


void write_tab(char *tab,FILE *fichier){
	int i=0;
	while(tab[i]!='\0'){
		fputc(tab[i],fichier);
		i++;
	}
}

int main (int argc, char *argv[]){

	if(argc != 3){
		printf("\n[Error] Format <file_from_cayenne.txt> <file_to_excel.txt>\n");
		return 1; 
	}

	c = argv[1][idx];
	while(argv[1][idx] != '\0'){
		idx++ ; 
	}
	if((argv[1][idx-3] != 't') || (argv[1][idx-2] != 'x') || (argv[1][idx-1] != 't')){
		printf("\n[Error] Extention file from cayenne must be \".txt\"\n");
		return 1; 
	}
	idx=0;

	c = argv[2][idx];
	while(argv[2][idx] != '\0'){
		idx++ ; 
	}
	if((argv[2][idx-3] != 't') || (argv[2][idx-2] != 'x') || (argv[2][idx-1] != 't')){
		printf("\n[Error] Extention file to excel must be \".txt\"\n");
		return 1; 
	}
	idx=0;


	in=fopen(argv[1],"r");
		if (in == NULL){
			printf("[Error] Cannot oppen <file_from_cayenne> \n");
			return 1;
		}
	out=fopen(argv[2],"w");
		if (out == NULL){
			printf("[Error] Cannot creat <file_to_excel> \n");
			return 1;
		}

//Exception for the first line 
	if(fgets(line,CHAR_MAX_LINE,in)==NULL){
		 printf("[Error] <file_from_cayenne> is empty\n");
	}

	do{
	  idx++;
	  c = line[idx];
	}while(c!='"');
	idx++;
	fputs("\"Date\",\"Time\"",out);

	while(line[idx] != '\0'){
		fputc(line[idx],out);
		idx++; //virgule
	}
	line[idx]='\n';

	while(fgets(line,CHAR_MAX_LINE,in)!=NULL){

		idx=0; 
		do{
	  		idx++;
	  		c = line[idx];
		}while(c!='"');
		idx++;

		for(int i=1;i<5;i++){
			year[i-1]=line[i];
		}
		
		for(int i=6; i<8; i++){
			month[i-6]=line[i];
		}
		
		for(int i=9;i<11;i++){
			day[i-9]=line[i];
		}
		
		for(int i=12;i<14;i++){
			h[i-12]=line[i];
		}
		
		for(int i=15;i<17;i++){
			m[i-15]=line[i];
		}
		
		for(int i=18;i<20;i++){
			s[i-18]=line[i];
		}

		fputc('"',out);write_tab(day,out);fputc('/',out);write_tab(month,out);fputc('/',out);write_tab(year,out);fputc('"',out);
		fputc(',',out);

		fputc('"',out);
		write_tab(h,out);
		fputc(':',out);
		write_tab(m,out);
		fputc(':',out);
		write_tab(s,out);
		fputc('"',out);

		while(line[idx] != '\0'){
			if(line[idx]=='.'){
				line[idx]=',';
			}
			fputc(line[idx],out);
			idx++; 
		}
		line[idx]='\n';


	}

	printf("Is done\nFile \"%s\" is creat\n",argv[2]); 

	fclose(in); 
	fclose(out); 

	return 0;
}