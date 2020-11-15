#include<myPrintf.h>
#include<drivers/Dev/dev.h>


void printFunction(char *str, int length)
{
    if((length < 0 ) || (length > OUT_MAX_LEN))
    {
        char *error_message = "sorry, the output sentence is invalid！\n";
        while(*error_message)
        {
            kernel_printCH(*error_message);
			error_message++;
        }
    }
    //终止
    if((length == 1) && (str[0] == '\0'))
        return;
    int j;
    for(j = 0; j < length; j++)
    {
        kernel_printCH(str[j]);
    }
}


void outNumFunctionP(unsigned long num, int outputWidth, int right, char alignCH, int isUp, int b, int flag)
{

	char reverseStr[101], outputStr[101];
	int j = 0;
	int i;
	int len = 0;
	do{
		int a = num % b;
		if(a <= 9)
		{
			reverseStr[j++] = a + '0';
		}
		else
		{
			if(isUp)
				reverseStr[j++] = a - 10 + 'A';
			else
				reverseStr[j++] = a - 10 + 'a';			
		}
		num /= b;
		len++;
	} while(num != 0);
	if(!flag)
	{
		reverseStr[j++] = '-';
		len++;
	}


	if(outputWidth < len)
		outputWidth = len;

	if(right && !flag)
	{
		if(alignCH == '0')
		{
			for(i = len - 1; i < outputWidth - 1; i++)
				reverseStr[i] = alignCH;
			reverseStr[outputWidth - 1] = '-';
			
		}
		else
		{
			for(i = len; i < outputWidth; i++)
				reverseStr[i] = alignCH;
		}
		
	}
	else
	{
		
		for(i = len; i < outputWidth; i++)
			reverseStr[i] = alignCH;
	}
	
	if(right)	
	{
		for(i = 0; i < outputWidth; i++)
			outputStr[i] = reverseStr[outputWidth - 1 - i];
	}
	else
	{
		
		for(i = 0; i < len; i++)
			outputStr[i] = reverseStr[len- 1 - i];
		for(i = len; i < outputWidth; i++)
			outputStr[i] = ' ';
	}	
	printFunction(outputStr, outputWidth);	
}


void myPrintf(char *input, ...)
{
	va_list va_args;
	va_start(va_args, input);
	char outp[OUT_MAX_LEN];

	long int tempNum;
	int isLong = 0;//默认为非long型的
	int outputWidth = 1;//输出宽度
	int flag = 1; //正数为1，负数为0
	int precision = 0; //有效位
	int length = 0;
	int right = 1;//默认靠右边输出
	char alignCH = ' ';
	int hasE = 0;

	while(1)
	{
		length = 0;
		char *begin = input;
		while((*input) && (*input != '%'))
		{
			input++;
			length++;
		}
		printFunction(begin, length);
		if(!(*input))
			break;

		//当前fmt为%
		if(*input == '%')
			input++;
		
		if(*input == '%')
		{
			input++;
			printFunction("%", 1);
			continue; 
		}

		
		hasE = 0;
		if(*input == '#')
		{
			input++;
			hasE = 1;
		}

		right = 1; 
		//是否靠左边输出
		if(*input == '-')
		{
			right = 0;
			input++;
		}

		alignCH = ' ';
		
		if(*input == '0')
		{
			alignCH = '0';
			input++;
		}
		char alignStr[3];
		alignStr[0] = alignCH;
		alignStr[1] = 0;

		
		
		outputWidth = 1;//默认的输出宽度为1
		//输出宽度可以为多位数
		if((*input >= '0') && (*input <= '9'))
		{
			outputWidth = (*input) - '0';
			input++;
			while((*input >= '0') && (*input <= '9'))
			{
				outputWidth = (*input) - '0' + 10 * outputWidth;
				input++;
			}
		}
		
		precision = -1;
		if(*input == '.')
		{
			*input++;
			if((*input >= '0') && (*input <= '9'))
			{
				precision = 0;
				while((*input >= '0') && (*input <= '9'))
				{
					precision = (*input) - '0' + 10 * precision;
					input++;
				}
			}
		}

		
		if(*input == 0)
		{
			break;
		}

		
		if(*input == 'c')
		{
			//va_arg返回值是int型的
			char c = (char)va_arg(va_args, int);
			int i;
			if(outputWidth < 1) 
				outputWidth = 1;
			if(right)
			{
				for(i = 0; i < outputWidth - 1; i++)
					printFunction(alignStr, 1);
				outp[0] = c;
				outp[1] = 0;
				printFunction(outp, 1);
			}
			else
			{
				outp[0] = c;
				outp[1] = 0;
				printFunction(outp, 1);
				for(i = 0; i < outputWidth - 1; i++)
					printFunction(" ", 1);
			}			
			input++;
			continue;
		}

		
		if(*input == 's')
		{
			char *tempStr = (char *)va_arg(va_args, char *);
			int l = 0;
			int i;
			char *sLen = tempStr;
			while(*sLen)
			{
				l++;
				sLen++;
			}
			if(outputWidth < l)
				outputWidth = l;
			if(right)
			{
				for(i = 0; i < outputWidth - l; i++)
				{
					outp[i] = alignCH;
				}
				int k;
				for(i = outputWidth - l, k = 0; i < outputWidth && k < l; i++, k++)
				{
					outp[i] = tempStr[k];
				}
				printFunction(outp, outputWidth);
			}
			else
			{
				for(i = 0; i < l; i++)
				{
					outp[i] = tempStr[i];
				}
				for(i = l; i < outputWidth; i++)
				{
					outp[i] = ' ';
				}
				printFunction(outp, outputWidth);
			}			
			input++;
			continue;
		}


		//判断是否为long型
		if(*input == 'l')
		{
			isLong = 1;
			input++;
		}
		else
		{
			isLong = 0;
		}


		
		//二进制
		if(*input == 'b')
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 0, 2, 1);
			input++;
			continue;
		}
		//八进制
		if((*input == 'o') || (*input) == 'O')
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			if(hasE)
				printFunction("0", 1);
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 0, 8, 1);
			input++;
			continue;
		}
		
		if((*input == 'u') || (*input) == 'U')
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 0, 10, 1);
			input++;
			continue;
		}
		//16进制区分字母的大小写
		if((*input == 'x'))
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			if(hasE)
				printFunction("0x", 2);
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 0, 16, 1);
			input++;
			continue;
		}

		if((*input == 'X'))
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			if(hasE)
				printFunction("0x", 2);
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 1, 16, 1);
			input++;
			continue;
		}

		//十进制处理负数
		if((*input == 'd') || (*input) == 'D')
		{
			if(isLong)
				tempNum = va_arg(va_args, long int);
			else
			{
				tempNum = va_arg(va_args, int);
			}
			if(tempNum < 0)
			{
				tempNum = -tempNum;
				flag = 0;
			}
			else
			{
				flag = 1;
			}			
			outNumFunctionP(tempNum, outputWidth, right, alignCH, 0, 10, flag);
			input++;
			continue;
		}	

	}
	//结束打印
	return;
}

void kernel_panic(char *curFile, int curLine)
{
    myPrintf("Here, kernel panic occurs at %s, %d \n", curFile, curLine);
    myPrintf(" \n!!!!!!!\n");
    while(1);
}

void myPrintfTest()
{
	myPrintf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	myPrintf("\n开始测试myPrintf功能\n\n");
	char ch = 't';
	myPrintf("打印字符测试\n");
	myPrintf("字符 t 按照 %%c     打印: %c\n", ch);
	myPrintf("字符 t 按照 %%12c   打印: %12c\n", ch);
	myPrintf("字符 t 按照 %%012c  打印: %012c\n", ch);
	myPrintf("字符 t 按照 %%-012c 打印: %-012c\n", ch);
	char *testStr = "test";
	myPrintf("打印字符串测试\n");
	myPrintf("字符串 test 按照 %%3s    打印: %3s\n", testStr);
	myPrintf("字符串 test 按照 %%s     打印: %s\n", testStr);
	myPrintf("字符串 test 按照 %%20s   打印: %20s\n", testStr);
	myPrintf("字符串 test 按照 %%020s  打印: %020s\n", testStr);
	myPrintf("字符串 test 按照 %%-020s 打印: %-020s\n", testStr);
	myPrintf("打印数字测试\n");
	myPrintf("2按照    %%4b  打印: %4b\n", 2);
	myPrintf("1000按照 %%5d  打印: %5d\n", 1000);
	myPrintf("8按照    %%2o  打印: %2o\n", 8);
	myPrintf("8按照    %%#o  打印: %#o\n", 8);
	myPrintf("28按照   %%4x  打印: %4x\n", 28);
	myPrintf("28按照   %%-4x  打印: %-4x\n", 28);
	myPrintf("28按照   %%04X  打印: %04X\n", 28);//1C = 16 + 12
	myPrintf("28按照   %%#08X  打印: %#08X\n", 28);
	myPrintf(" 20按照  %%9d  打印: %9d\n", 20);
	myPrintf(" 20按照  %%09d 打印: %09d\n", 20);
	myPrintf("-20按照  %%09d 打印: %09d\n", -20);
	myPrintf("20按照  %%-09d 打印: %-09d\n", 20);
	myPrintf("-20按照 %%-09d 打印: %-09d\n", -20);
	myPrintf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	return;
}
