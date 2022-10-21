#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/*
    读取输入字符串中的操作数
    返回读取后的指针
    若输入不合法则返回null。
*/
char* read_op(char* str, char* op, bool* flag){
    if((str[0]<'0'||str[0]>'9')&&str[0]!='-')return NULL;
    *flag=true;
    if(str[0]=='-'){
        str++;
        *flag=false;
    }
    int i=0;
    while(*str>='0'&&*str<='9'){
        op[i]=*str;
        i++;
        str++;
    }
    op[i] = '\0';
    return str;
}

/*
    op2 = op1 + op2
    输入，输出均为逆序数
*/
void add(char* op1, char* op2){
    int l1 = strlen(op1);
    int l2 = strlen(op2);
    int i = 0;
    int carry = 0;
    while (i<l1&&i<l2) {
        int tem = carry + (op1[i] - '0') + (op2[i] - '0');
        op2[i] = (tem%10) + '0';
        carry = tem / 10;
        i++;
    }
    while (i<l1){
        int tem = carry + (op1[i] - '0');
        op2[i] = (tem%10) + '0';
        carry = tem / 10;
        i++;
    }
    while (i<l2){
        int tem = carry + (op2[i] - '0');
        op2[i] = (tem%10) + '0';
        carry = tem / 10;
        i++;
    }
    if(carry>0){
        op2[i] = carry+'0';
        i++;
    }
    op2[i] = '\0';
}
/*
比较两个正序字符串数，op1>op2 -> 1, op1<op2 ->-1, op1==op2 ->0
*/
int cmp (char* op1, char* op2){
    int l1 = strlen(op1);
    int l2 = strlen(op2);
    if(l1!=l2){
        if(l1>l2)return 1;
        else return -1;
    }
    else{
        for(int i=0;i<l1;i++){
            if(op1[i]>op2[i])return 1;
            if(op1[i]<op2[i])return -1;
        }
        return 0;
    }
}

/*
两个逆序正数相减,要求op1>op2, op1 = op1 - op2
*/
void sub(char* op1, char* op2){
    int l1 = strlen(op1);
    int l2 = strlen(op2);
    int i=0;
    for(;i<l2;i++){
        if(op1[i]>=op2[i]){
            op1[i] = (op1[i]-op2[i])+'0';
        }
        else{//借位
            op1[i+1]--;
            op1[i] = ((10+op1[i])-op2[i])+'0';
        }
    }
    while(op1[i]<'0'&&op1[i]>0){
        op1[i] = '9';
        op1[i+1] --;
        i++;
    }
}


void mul(char* op1, char* op2, char* res){
    char tem[100];
    memset(tem, '0', 100);
    memset(res, 0, 100);
    res[0] = '0';
    int l1 = strlen(op1);
    int l2 = strlen(op2);
    int carry = 0;
    for(int i=0; i<l1; i++){
        for(int j=0; j<l2; j++){
            int s = (op1[i]-'0')*(op2[j]-'0')+carry;
            tem[i+j] = s%10+'0';
            carry = s/10;
        }
        if(carry>0){
            tem[i+l2] = carry+'0';
            tem[i+l2+1] = '\0';
        }else{
            tem[i+l2] = '\0';
        }
        add(tem, res);
        memset(tem, '0', 100);
        carry = 0;
    }
}

void reverse(char* str){
    int l = 0;
    int r = strlen(str)-1;
    while (l<r) {
        char t = str[l];
        str[l] = str[r];
        str[r] = t;
        l++;
        r--;
    }
}

int main(){
    char op1[30], op2[30];
    char str[100];
    char res[100];
    bool flag1, flag2;
    char ope;
    char* p = str;
    while(1){
        scanf("%s", str);
        p = str;
        if(str[0]=='q'){
            break;
        }
        else{
            p = read_op(p, op1, &flag1);
            if(p==NULL){
                printf("invalid\n");
                continue;
            }
            ope = *p;
            if(ope!='+'&&ope!='*'){
                printf("invalid\n");
                continue;
            } 
            p++;
            p = read_op(p, op2, &flag2);
            if(p==NULL){
                printf("invalid\n");
                continue;
            }
            if(*p!='\0'){
                printf("invalid\n");
                continue;
            }
            switch (ope) {
                case '+':
                    if(flag1==flag2){
                        reverse(op1);
                        reverse(op2);
                        add(op1, op2);
                        if(!flag1)printf("-");
                        reverse(op2);
                        printf("%s\n", op2);
                        break;
                    }else{
                        int cmp_result = cmp(op1, op2);
                        bool ans_flag = flag1;
                        if(cmp_result==0){
                            printf("0\n");
                            continue;
                        }
                        char* p1 = op1;
                        char* p2 = op2;
                        if(cmp_result==-1){
                            ans_flag = flag2;
                            char* t = p1;
                            p1 = p2;
                            p2 = t;
                        }
                        reverse(p1);
                        reverse(p2);
                        sub(p1, p2);
                        reverse(p1);
                        if(!ans_flag)printf("-");
                        printf("%s\n", p1);
                        break;
                    }
                        
                case '*':
                    reverse(op1);
                    reverse(op2);
                    mul(op1, op2, res);
                    if(flag1^flag2)printf("-");
                    reverse(res);
                    printf("%s\n", res);
                    break;

                default:
                    printf("invalid\n");
                    continue;
            } 
        }
    }
    return 0;

}