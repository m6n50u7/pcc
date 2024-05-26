#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

int hexchar2int(char hex)
{
	if(hex == '0')
	return 0;
	if(hex == '1')
	return 1;
	if(hex == '2')
	return 2;
	if(hex == '3')
	return 3;
	if(hex == '4')
	return 4;
	if(hex == '5')
	return 5;
	if(hex == '6')
	return 6;
	if(hex == '7')
	return 7;
	if(hex == '8')
	return 8;
	if(hex == '9')
	return 9;
	if(hex == 'a' || hex == 'A')
	return 10;
	if(hex == 'b' || hex == 'B')
	return 11;
	if(hex == 'c' || hex == 'C')
	return 12;
	if(hex == 'd' || hex == 'D')
	return 13;
	if(hex == 'e' || hex == 'E')
	return 14;
	if(hex == 'f' || hex == 'F')
	return 15;
	return 16;
}
int length(char string[])
{
	int i;
	for(i = 0; string[i] != '\0'; i++){}
	return i;
}
int hexchars2int(char hex[], int i)
{
	return hexchar2int(hex[i * 2]) * 16 + hexchar2int(hex[i * 2 + 1]);
}
int str2int(char string[])
{
	int num=0, power = 1;
	int ln=length(string);
	for(int i = ln; i > 0; i--)
	{
		if(string[ln - i] != '-')
		{
			num += power * char2int(string[ln - i]);
			power *= 10;
		}
	}
	if(string[0] == '-')
		return -num;
	else
		return num;
}


int csum, tsum, STol, GTol, siz, size;
unsigned char clr[3], trgt[3];
unsigned char *data;
signed char spectr(unsigned char input[3])
{
	//green = 0, red = -85, blue = 85
	unsigned char tmp[2];
	//check if color is near green
	if(input[1] >= input[0] && input[1] <= input[2])
	{
		if(input[0] == input[2])
			return 0;
		if(input[2] > input[0])
		{
			tmp[0] = input[1] - input[0];//new green
			tmp[1] = input[2] - input[0];//new blue
			return (85 * tmp[1]) / (tmp[0] + tmp[1]);
		}
		else
		{
			tmp[0] = input[1] - input[2];//new green
			tmp[1] = input[0] - input[2];//new red
			return (-85 * tmp[1]) / (tmp[0] + tmp[1]);
		}
	}
	else
	{
		tmp[0] = input[0] - input[1];//new red
		tmp[1] = input[2] - input[1];//new blue
		return 85 + (tmp[0] * 85) / (tmp[0] + tmp[1]);
	}
}
int checkTol(unsigned char input[3])
{
	//if tolerance for both grayscale and specter are maximum, this means entire picture is valid for Color Conversion. directly accept to optimize the code.
	if(GTol == 255 && STol >= 127)
		return 1;
	if(GTol != 255)
	{
		int gray = (input[0] * 30 + input[1] * 59 + input[2] * 11) / 100;
		int t = tsum / 100;
		if(gray > (t + GTol) || gray < (t - GTol))
			return 0;
	}
	//the grayscale is adjacent to the entire color spectrum
	if(STol >= 127 || (input[0] == input[1] && input[0] == input[2]) || (trgt[0] == trgt[1] && trgt[0] == trgt[2]))
		return 1;
	signed char up, down, mid;
	mid = spectr(trgt);
	up = spectr(input) + STol;
	down = spectr(input) - STol;
	if(up > down)
		return mid > down && mid < up;
	return mid > down || mid < up;
}
unsigned char cnv(int clr, int tsum2)
{
	int ret = (clr * tsum2) / tsum;
	if(ret > 255)
		return 255;
	if(ret < 0)
		return 0;
	return (char) ret;
}
void cnvarr(unsigned char* input)
{
	if(checkTol(input))
	{
		int tsum2 = 1 + input[0] * 30 + input[1] * 59 + input[2] * 11;
		input[0] = cnv(clr[0], tsum2);
		input[1] = cnv(clr[1], tsum2);
		input[2] = cnv(clr[2], tsum2);
	}
}

// use multithreading to accelerate the work
int tnum = 1;

void *mtp(void *vargp) 
{
	pthread_t thread_id;
	unsigned char *tmpdst = data + tnum * 3;
	if(tnum < (DATAJUMP / 3))
		pthread_create(&thread_id, NULL, mtp, NULL);
	tnum++;
	long int rest = (size % DATAJUMP) / 3;
	for(long int curs = 0; curs < siz; curs++)
	{
		cnvarr(tmpdst);
		tmpdst += DATAJUMP;
	}
	pthread_join(thread_id, NULL);
	return NULL; 
} 

void convert(char* file)
{
	FILE *fptr1 = fopen(file, "rb+");
	fseek(fptr1, 0, SEEK_END);
	long int size = ftell(fptr1);
	fseek(fptr1, 0, SEEK_SET);
	data = malloc(size);
	unsigned char *tmpdst = data;
	fread(data, size, 1, fptr1);
	siz = size / DATAJUMP;
	pthread_t thread_id;
	if(tnum < (DATAJUMP / 3))
		pthread_create(&thread_id, NULL, mtp, NULL);
	long int rest = (size % DATAJUMP) / 3;
	for(long int curs = 0; curs < siz; curs++)
	{
		cnvarr(tmpdst);
		tmpdst += DATAJUMP;
	}
	pthread_join(thread_id, NULL);
	for(long int curs = 0; curs < rest; curs++)
	{
		cnvarr(tmpdst);
		tmpdst += 3;
	}
	fseek(fptr1, 0, SEEK_SET);
	fwrite(data, size, 1, fptr1);
	fclose(fptr1);
}

int main(int argn, char* args[])
{
	if(argn != 6)
	{
		printf("convert a color in RGB image to another color. intended as helper of IMPs.\nusage:\n%s IMAGE.rgb SRCRGB DSTRGB LIGHT_TOLERANCE COLOR_TOLLERANCE\nexample:\n%s test.rgb 2f34ae 1f249e 15 10\n", args[0], args[0]);
		return 1;
	}
	clr[0] = hexchars2int(args[2], 0);
	trgt[0] = hexchars2int(args[3], 0);
	clr[1] = hexchars2int(args[2], 1);
	trgt[1] = hexchars2int(args[3], 1);
	clr[2] = hexchars2int(args[2], 2);
	trgt[2] = hexchars2int(args[3], 2);
	GTol = str2int(args[4]);
	STol = str2int(args[5]);
	csum = 1 + clr[0] * 30 + clr[1] * 59 + clr[2] * 11;
	tsum = 1 + trgt[0] * 30 + trgt[1] * 59 + trgt[2] * 11;
	convert(args[1]);
	return 0;
}