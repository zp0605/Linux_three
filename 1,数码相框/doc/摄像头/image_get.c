#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>

//arm-linux-gcc image_get.c -o image_get-arm
//以下程序用于将jpg格式图片转化为bmp格式，15-585行号
  #define   PI   3.1415927   
  #define   widthbytes(i)   ((i+31)/32*4)   
    
  short   sampleYH,sampleYV,sampleUH,sampleUV,sampleVH,sampleVV;   
  short   HYtoU,VYtoU,HYtoV,VYtoV,YinMCU,UinMCU,VinMCU;   
  short   compressnum=0,Qt[3][64],*YQt,*UQt,*VQt,codepos[4][16],codelen[4][16];   
  unsigned   char   compressindex[3],YDCindex,YACindex,UVDCindex,UVACindex;   
  unsigned   char   HufTabindex,And[9]={0,1,3,7,0xf,0x1f,0x3f,0x7f,0xff};   
  unsigned   short   codevalue[4][256],hufmax[4][16],hufmin[4][16];   
  short   bitpos=0,curbyte=0,run=0,value=0,MCUbuffer[10*64],blockbuffer[64];   
  short   ycoef=0,ucoef=0,vcoef=0,intervalflag=0,interval=0,restart=0;   
  int   Y[4*64],U[4*64],V[4*64],QtZMCUbuffer[10*64];   
  unsigned   int   imgwidth=0,imgheight=0,width=0,height=0,linebytes;   
  short   Z[8][8]={{0,1,5,6,14,15,27,28},{2,4,7,13,16,26,29,42},   
            {3,8,12,17,25,30,41,43},{9,11,18,24,31,40,44,53},   
                            {10,19,23,32,39,45,52,54},{20,22,33,38,46,51,55,60},   
                            {21,34,37,47,50,56,59,61},{35,36,48,49,57,58,62,63}};   
    
  struct HEAD{   
    unsigned int   size;
    unsigned int   reserved;
    unsigned int   offset;
    }head;

  struct BMP{   
    unsigned int   size;   
    unsigned int   width;   
    unsigned int   height;   
    unsigned short   plane;   
    unsigned short   bitcount;   
    unsigned int   compression;   
    unsigned int   imagesize;   
    unsigned int   xpels;   
    unsigned int   ypels;   
    unsigned int   colorused;   
    unsigned int   colorimportant;   
    }bmp; 
    
  void   error(char   *s)   
    {   
      printf("%s\n",s);   
      exit(1);   
    }   
    
    
  void   makebmpheader(FILE   *fp)   
    {   
      unsigned short   i,j;   
      unsigned int   colorbits,imagebytes;   
      unsigned short  type=0x4d42;
      colorbits=24;
      linebytes=widthbytes(colorbits*imgwidth);   
      imagebytes=(unsigned int)imgheight*linebytes;   
      fwrite(&type,sizeof(type),1,fp);
      head.size=imagebytes+0x36;
      head.reserved=0;
      head.offset=0x36;
      fwrite(&head,sizeof(head),1,fp);
      bmp.size=0x28;
      bmp.width=(int)imgwidth;
      bmp.height=(int)imgheight;
      bmp.plane=1L;
      bmp.bitcount=colorbits;
      bmp.compression=0;
      bmp.imagesize=imagebytes;
      bmp.xpels=0xece;
      bmp.ypels=0xec4;
      bmp.colorused=0;
      bmp.colorimportant=0;
      fwrite(&bmp,sizeof(bmp),1,fp);
      for(j=0;j<imgheight;j++)
        for(i=0;i<linebytes;i++)
          fputc(0,fp);
    }
    
  void   initialize(FILE   *fp)   
    {   
      unsigned   char   *p,*q,hfindex,qtindex,number;   
      short   i,j,k,finish=0,huftab1,huftab2,huftabindex,count;   
      unsigned   short   length,flag;   
      fread(&flag,sizeof(unsigned   short),1,fp);   
      printf("%x\n",flag);
      if(flag!=0xd8ff)   
        error("Error   Jpg   File   format!1");   
      while(!finish)     {   
        fread(&flag,sizeof(unsigned   short),1,fp);   
        fread(&length,sizeof(short),1,fp);   
        length=((length<<8)|(length>>8))-2;   
        switch(flag)     {   
          case   0xe0ff:   
    fseek(fp,length,1);break;   
          case   0xdbff:   
    p=(unsigned char*)malloc(length);
    fread(p,length,1,fp);   
    qtindex=(*p)&0x0f;   
    q=p+1;   
    if(length+2<80)   
      for(i=0;i<64;i++)   
        Qt[qtindex][i]=(short)*(q++);   
    else     {   
      for(i=0;i<64;i++)   
        Qt[qtindex][i]=(short)*(q++);   
      qtindex=*(q++)&0x0f;   
      for(i=0;i<64;i++)   
        Qt[qtindex][i]=(short)*(q++);   
    }   
                    free(p);break;   
          case   0xc0ff:   
    p=(unsigned char*)malloc(length);   
    fread(p,length,1,fp);   
    imgheight=((*(p+1))<<8)+(*(p+2));   
    imgwidth=((*(p+3))<<8)+(*(p+4));   
    compressnum=*(p+5);   
    if((compressnum!=1)&&(compressnum!=3))   
      error("Error   Jpg   File   format!2");   
    if(compressnum==3)     {   
      compressindex[0]=*(p+6);   
      sampleYH=(*(p+7))>>4;   
      sampleYV=(*(p+7))&0x0f;   
      YQt=(short   *)Qt[*(p+8)];   
      compressindex[1]=*(p+9);   
      sampleUH=(*(p+10))>>4;   
      sampleUV=(*(p+10))&0x0f;   
      UQt=(short   *)Qt[*(p+11)];   
      compressindex[2]=*(p+12);   
      sampleVH=(*(p+13))>>4;   
      sampleVV=(*(p+13))&0x0f;   
      VQt=(short   *)Qt[*(p+14)];   
    }   
                    else     {   
      compressindex[0]=*(p+6);   
      sampleYH=(*(p+7))>>4;   
      sampleYV=(*(p+7))&0x0f;   
      YQt=(short   *)Qt[*(p+8)];   
                      compressindex[1]=*(p+6);   
                      sampleUH=1;   
                      sampleUV=1;   
      UQt=(short   *)Qt[*(p+8)];   
                      compressindex[2]=*(p+6);   
                      sampleVH=1;   
                      sampleVV=1;   
      VQt=(short   *)Qt[*(p+8)];   
    }   
                    free(p);break;   
          case   0xc4ff:   
    p=(unsigned char*)malloc(length+1);   
    fread(p,length,1,fp);   
    p[length]=0xff;   
    if(length+2<0xd0)     {   
      huftab1=(short)(*p)>>4;   
      huftab2=(short)(*p)&0x0f;   
      huftabindex=huftab1*2+huftab2;   
      q=p+1;   
      for(i=0;i<16;i++)   
        codelen[huftabindex][i]=(short)(*(q++));   
      j=0;   
      for(i=0;i<16;i++)   
        if(codelen[huftabindex][i]!=0)     {   
          k=0;   
          while(k<codelen[huftabindex][i])     {   
            codevalue[huftabindex][k+j]=(short)(*(q++));   
            k++;   
          }   
          j+=k;   
        }   
        i=0;   
        while(codelen[huftabindex][i]==0)   i++;   
        for(j=0;j<i;j++)     {   
          hufmin[huftabindex][j]=0;   
          hufmax[huftabindex][j]=0;   
        }   
        hufmin[huftabindex][i]=0;   
        hufmax[huftabindex][i]=codelen[huftabindex][i]-1;   
        for(j=i+1;j<16;j++)     {   
          hufmin[huftabindex][j]=(hufmax[huftabindex][j-1]+1)<<1;   
          hufmax[huftabindex][j]=hufmin[huftabindex][j]+codelen[huftabindex][j]-1;   
        }   
        codepos[huftabindex][0]=0;   
        for(j=1;j<16;j++)   
          codepos[huftabindex][j]=codelen[huftabindex][j-1]+codepos[huftabindex][j-1];   
    }   
    else     {   
      hfindex=*p;   
      while(hfindex!=0xff)     {   
        huftab1=(short)hfindex>>4;   
        huftab2=(short)hfindex&0x0f;   
        huftabindex=huftab1*2+huftab2;   
        q=p+1;   
        count=0;   
        for(i=0;i<16;i++)     {   
          codelen[huftabindex][i]=(short)(*(q++));   
          count+=codelen[huftabindex][i];   
        }   
        count+=17;   
        j=0;   
        for(i=0;i<16;i++)   
          if(codelen[huftabindex][i]!=0)   {   
            k=0;   
            while(k<codelen[huftabindex][i])     {   
              codevalue[huftabindex][k+j]=(short)(*(q++));   
              k++;   
            }   
            j+=k;   
          }   
          i=0;   
          while(codelen[huftabindex][i]==0)     i++;   
          for(j=0;j<i;j++)     {   
            hufmin[huftabindex][j]=0;   
            hufmax[huftabindex][j]=0;   
          }   
          hufmin[huftabindex][i]=0;   
          hufmax[huftabindex][i]=codelen[huftabindex][i]-1;   
          for(j=i+1;j<16;j++)     {   
            hufmin[huftabindex][j]=(hufmax[huftabindex][j-1]+1)<<1;   
            hufmax[huftabindex][j]=hufmin[huftabindex][j]+codelen[huftabindex][j]-1;   
          }   
          codepos[huftabindex][0]=0;   
          for(j=1;j<16;j++)   
            codepos[huftabindex][j]=codelen[huftabindex][j-1]+codepos[huftabindex][j-1];   
          p+=count;   
          hfindex=*p;   
      }   
      p-=length;   
    }   
    free(p);break;   
          case   0xddff:   
    p=(unsigned char*)malloc(length);   
    fread(p,length,1,fp);   
                    restart=((*p)<<8)|(*(p+1));   
                    free(p);break;   
          case   0xdaff:   
    p=(unsigned char*)malloc(length*sizeof(unsigned   char));   
    fread(p,length,1,fp);   
    number=*p;   
    if(number!=compressnum)   
      error("Error   Jpg   File   format!3");   
    q=p+1;   
    for(i=0;i<compressnum;i++)     {   
      if(*q==compressindex[0])     {   
        YDCindex=(*(q+1))>>4;   
        YACindex=((*(q+1))&0x0f)+2;   
      }   
      else   {   
        UVDCindex=(*(q+1))>>4;   
        UVACindex=((*(q+1))&0x0f)+2;   
      }   
      q+=2;   
    }   
    finish=1;   
                    free(p);break;   
          case   0xd9ff:   
    error("Error   Jpg   File   format!4");break;   
          default:   
                    if((flag&0xf000)!=0xd000)   
      fseek(fp,length,1);   
                    break;   
        }   
      }   
    }    
    
  void   savebmp(FILE   *fp)   
    {   
      short   i,j;   
      unsigned   char   r,g,b;   
      int   y,u,v,rr,gg,bb;   
      for(i=0;i<sampleYV*8;i++)     {   
        if((height+i)<imgheight)     {   
          fseek(fp,(unsigned   int)(imgheight-height-i-1)*linebytes+3*width+54,0);   
          for(j=0;j<sampleYH*8;j++)     {   
            if((width+j)<imgwidth)     {   
              y=Y[i*8*sampleYH+j];   
              u=U[(i/VYtoU)*8*sampleYH+j/HYtoU];   
              v=V[(i/VYtoV)*8*sampleYH+j/HYtoV];   
              /*rr=y+((359*v)>>8;
              gg=y-((88*u-183*v)>>8);
              bb=((y<<8)+301*u)>>8; */
              rr=(int)(y+1.402*v);
              gg=(int)(y-0.34414*u-0.71414*v);
              bb=(int)(y+1.772*u);
              r=(unsigned   char)rr;   
              g=(unsigned   char)gg;   
              b=(unsigned   char)bb;   
              if(rr&0xffffff00)   if   (rr>255)   r=255;   else   if   (rr<0)   r=0;   
              if(gg&0xffffff00)   if   (gg>255)   g=255;   else   if   (gg<0)   g=0;   
              if(bb&0xffffff00)   if   (bb>255)   b=255;   else   if   (bb<0)   b=0;   
              fputc(b,fp);fputc(g,fp);fputc(r,fp);   
            }   
          else     break;   
        }   
      }   
      else   break;   
      }   
    }   
    
  unsigned   char   readbyte(FILE   *fp)   
    {   
      unsigned   char   c;   
      c=fgetc(fp);   
      if(c==0xff)   
        fgetc(fp);   
      bitpos=8;   
      curbyte=c;   
      return   c;   
    }   
    
  short   DecodeElement(FILE   *fp)   
    {   
      short   codelength;   
      int   thiscode,tempcode;   
      unsigned short new,temp;   
      unsigned char hufbyte,runsize,tempsize,sign;   
      unsigned char newbyte,lastbyte;   
      if(bitpos>=1)   {   
        bitpos--;   
        thiscode=(unsigned   char)curbyte>>bitpos;   
        curbyte=curbyte&And[bitpos];   
      }   
      else     {   
        lastbyte=readbyte(fp);   
        bitpos--;   
        newbyte=curbyte&And[bitpos];   
        thiscode=lastbyte>>7;   
        curbyte=newbyte;   
      }   
      codelength=1;   
      while((thiscode<hufmin[HufTabindex][codelength-1])||   
  (codelen[HufTabindex][codelength-1]==0)||   
  (thiscode>hufmax[HufTabindex][codelength-1]))     {   
        if(bitpos>=1)     {   
          bitpos--;   
          tempcode=(unsigned   char)curbyte>>bitpos;   
          curbyte=curbyte&And[bitpos];   
        }   
        else     {   
          lastbyte=readbyte(fp);   
          bitpos--;   
          newbyte=curbyte&And[bitpos];   
          tempcode=(unsigned   char)lastbyte>>7;   
          curbyte=newbyte;   
        }   
        thiscode=(thiscode<<1)+tempcode;   
        codelength++;   
        if(codelength>16)   
          error("Error   Jpg   File   format!");   
      }   
      temp=thiscode-hufmin[HufTabindex][codelength-1]+codepos[HufTabindex][codelength-1];   
      hufbyte=(unsigned   char)codevalue[HufTabindex][temp];   
      run=(short)(hufbyte>>4);   
      runsize=hufbyte&0x0f;   
      if(runsize==0)     {   
        value=0;   
        return   1;   
      }   
      tempsize=runsize;   
      if(bitpos>=runsize)     {   
        bitpos-=runsize;
        new=(unsigned   char)curbyte>>bitpos;   
        curbyte=curbyte&And[bitpos];   
      }   
      else     {   
        new=curbyte;   
        tempsize-=bitpos;   
        while(tempsize>8)     {   
          lastbyte=readbyte(fp);   
          new=(new<<8)+(unsigned   char)lastbyte;   
          tempsize-=8;   
        }   
        lastbyte=readbyte(fp);   
        bitpos-=tempsize;   
        new=(new<<tempsize)+(lastbyte>>bitpos);   
        curbyte=lastbyte&And[bitpos];   
      }   
      sign=new>>(runsize-1);   
      if(sign)   
        value=new;
      else     {   
        new=new^0xffff;   
        temp=0xffff<<runsize;   
        value=-(short)(new^temp);   
      }   
      return   1;   
    }   
    
  short   HufBlock(FILE   *fp,unsigned   char   dchufindex,   
                            unsigned   char   achufindex)   
    {   
      short   i,count=0;   
      HufTabindex=dchufindex;   
      if(DecodeElement(fp)!=1)   
        return   0;   
      blockbuffer[count++]=value;   
      HufTabindex=achufindex;   
      while   (count<64)     {   
        if(DecodeElement(fp)!=1)   
          return   0;   
        if((run==0)&&(value==0))     {   
          for(i=count;i<64;i++)   
            blockbuffer[i]=0;   
          count=64;   
        }   
        else     {   
          for(i=0;i<run;i++)   
            blockbuffer[count++]=0;   
          blockbuffer[count++]=value;   
        }   
      }   
      return   1;   
    }   
    
  short   DecodeMCUBlock(FILE   *fp)   
    {   
      short   i,j,*pMCUBuffer;   
      if(intervalflag)     {   
        fseek(fp,2,1);   
        ycoef=ucoef=vcoef=0;   
        bitpos=0;   
        curbyte=0;   
      }   
      switch(compressnum)     {   
        case   3:   
                      pMCUBuffer=MCUbuffer;   
                      for(i=0;i<sampleYH*sampleYV;i++)     {   
                        if(HufBlock(fp,YDCindex,YACindex)!=1)   
                          return   0;   
                        blockbuffer[0]=blockbuffer[0]+ycoef;   
                        ycoef=blockbuffer[0];   
                        for(j=0;j<64;j++)   
                          *pMCUBuffer++=blockbuffer[j];   
                      }   
                      for(i=0;i<sampleUH*sampleUV;i++)     {   
                        if(HufBlock(fp,UVDCindex,UVACindex)!=1)   
                          return   0;   
                        blockbuffer[0]=blockbuffer[0]+ucoef;   
                        ucoef=blockbuffer[0];   
                        for(j=0;j<64;j++)   
                          *pMCUBuffer++=blockbuffer[j];   
                      }   
                      for(i=0;i<sampleVH*sampleVV;i++)     {   
        if(HufBlock(fp,UVDCindex,UVACindex)!=1)   
                          return   0;   
                        blockbuffer[0]=blockbuffer[0]+vcoef;   
                        vcoef=blockbuffer[0];   
                        for(j=0;j<64;j++)   
                          *pMCUBuffer++=blockbuffer[j];   
                      }   
                      break;   
        case   1:   
                      pMCUBuffer=MCUbuffer;   
      if(HufBlock(fp,YDCindex,YACindex)!=1)   
                        return   0;   
                      blockbuffer[0]=blockbuffer[0]+ycoef;   
                      ycoef=blockbuffer[0];   
                      for(j=0;j<64;j++)   
                        *pMCUBuffer++=blockbuffer[j];   
                      for(i=0;i<128;i++)   
                        *pMCUBuffer++=0;   
                      break;   
        default:   
                        error("Error   Jpg   File   format");   
      }   
      return(1);   
    }   
    
  void   idct(int   *p,short   k)   
    {   
      int   x,x0,x1,x2,x3,x4,x5,x6,x7;   
      x1=p[k*4]<<11;x2=p[k*6];x3=p[k*2];x4=p[k*1];   
      x5=p[k*7];x6=p[k*5];x7=p[k*3];x0=(p[0]<<11)+1024;   
      x=565*(x4+x5);x4=x+2276*x4;x5=x-3406*x5;   
      x=2408*(x6+x7);x6=x-799*x6;x7=x-4017*x7;   
      x=1108*(x3+x2);x2=x-3784*x2;x3=x+1568*x3;   
      x=x6;x6=x5+x7;x5-=x7;x7=x0+x1;x0-=x1;x1=x+x4;x4-=x;   
      x=x5;x5=x7-x3;x7+=x3;x3=x0+x2;x0-=x2;   
      x2=(181*(x4+x)+128)>>8;x4=(181*(x4-x)+128)>>8;   
      p[0]=(x7+x1)>>11;p[k*1]=(x3+x2)>>11;   
      p[k*2]=(x0+x4)>>11;p[k*3]=(x5+x6)>>11;   
      p[k*4]=(x5-x6)>>11;p[k*5]=(x0-x4)>>11;   
      p[k*6]=(x3-x2)>>11;p[k*7]=(x7-x1)>>11;   
    }   
    
  void   IDCTint(int   *metrix)   
    {   
      short   i;   
      for(i=0;i<8;i++)   
        idct(metrix+8*i,1);   
      for(i=0;i<8;i++)   
        idct(metrix+i,8);   
    }   
    
  void   IQtZBlock(short   *s,int   *d,short   *pQt,short   correct)   
    {   
      short   i,j,tag;   
      int   *pbuffer,buffer[8][8];   
      for(i=0;i<8;i++)   
        for(j=0;j<8;j++)   {   
          tag=Z[i][j];   
          buffer[i][j]=(int)s[tag]*(int)pQt[tag];   
        }   
      pbuffer=(int   *)buffer;   
      IDCTint(pbuffer);   
      for(i=0;i<8;i++)   
        for(j=0;j<8;j++)   
          d[i*8+j]=(buffer[i][j]>>3)+correct;   
    }   
    
  void   IQtZMCU(short   xx,short   yy,short   offset,short   *pQt,short   correct)   
    {   
      short   i,j,*pMCUBuffer;   
      int   *pQtZMCUBuffer;   
      pMCUBuffer=MCUbuffer+offset;   
      pQtZMCUBuffer=QtZMCUbuffer+offset;   
      for(i=0;i<yy;i++)   
        for(j=0;j<xx;j++)   
          IQtZBlock(pMCUBuffer+(i*xx+j)*64,pQtZMCUBuffer+(i*xx+j)*64,pQt,correct);   
    }   
    
  void   getYUV(short   xx,short   yy,int   *buf,short   offset)   
    {   
      short   i,j,k,n;   
      int   *pQtZMCU;   
      pQtZMCU=QtZMCUbuffer+offset;   
      for(i=0;i<yy;i++)   
        for(j=0;j<xx;j++)   
          for(k=0;k<8;k++)   
            for(n=0;n<8;n++)   
              buf[(i*8+k)*sampleYH*8+j*8+n]=*pQtZMCU++;   
    }   
    
  void   decode(FILE   *fp1,FILE   *fp2)   
    {   
      short   Yinbuf,Uinbuf,Vinbuf;   
      YinMCU=sampleYH*sampleYV;   
      UinMCU=sampleUH*sampleUV;   
      VinMCU=sampleVH*sampleVV;   
      HYtoU=sampleYH/sampleUH;   
      VYtoU=sampleYV/sampleUV;   
      HYtoV=sampleYH/sampleVH;   
      VYtoV=sampleYV/sampleVV;   
      Yinbuf=0;Uinbuf=YinMCU*64;   
      Vinbuf=(YinMCU+UinMCU)*64;   
      while(DecodeMCUBlock(fp1))     {   
        interval++;   
        if((restart)&&(interval%restart==0))   
          intervalflag=1;   
        else   
          intervalflag=0;   
        IQtZMCU(sampleYH,sampleYV,Yinbuf,YQt,128);   
        IQtZMCU(sampleUH,sampleUV,Uinbuf,UQt,0);   
        IQtZMCU(sampleVH,sampleVV,Vinbuf,VQt,0);   
        getYUV(sampleYH,sampleYV,Y,Yinbuf);   
        getYUV(sampleUH,sampleUV,U,Uinbuf);   
        getYUV(sampleVH,sampleVV,V,Vinbuf);   
        savebmp(fp2);   
        width+=sampleYH*8;   
        if(width>=imgwidth)   {   
          width=0;   
          height+=sampleYV*8;   
        }   
        if((width==0)&&(height>=imgheight))   
          break;   
      }   
    }   
    
int  jpg2bmp(FILE *fp1,FILE *fp2)
    {
      initialize(fp1);
      makebmpheader(fp2);decode(fp1,fp2);
      return 0;
    }


//以下程序用于从摄像头获取JPG格式数据

#define JPG      "/opt/video/image.jpg"
#define BMP      "/opt/video/image.bmp"
#define FILE_VIDEO "/dev/video2"


int main()

{
	int  fd;
	struct   v4l2_fmtdesc fmt;
	struct   v4l2_capability   cap;
	struct   v4l2_format      tv4l2_format;
	struct   v4l2_requestbuffers tV4L2_reqbuf;
	struct   v4l2_buffer tV4L2buf;
	enum     v4l2_buf_type v4l2type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fd_set    fds ; 
	struct timeval   tv;

     FILE *fp1,*fp2;
     int jpgsize=25600;
     int bmpsize=921654;
     unsigned char *buffer=NULL,*p;
     int i;

	buffer=(unsigned char *)malloc(jpgsize);
     if (buffer==NULL) {
         perror("Out of memory.\n");
         exit(1);
     }

     //Open video device
  fd = open(FILE_VIDEO, O_RDONLY);
     if (fd < 0)
         perror(FILE_VIDEO);
	//获取支持的视频格式
	memset(&fmt, 0, sizeof(fmt));
       fmt.index = 0;
       fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
              fmt.index++;
              printf("pixelformat = ''%c%c%c%c''\ndescription = ''%s''\n",fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,fmt.description);
       }
     //Get capabilities
     if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
         perror("VIDIOGCAP");
         printf("(" FILE_VIDEO " not a video4linux device?)\n");
         close(fd);
     }
    printf("capabilities-->%x\n",cap.capabilities);
/* Values for 'capabilities' field */
#define V4L2_CAP_VIDEO_CAPTURE  0x00000001  /* Is a video capture device */
#define V4L2_CAP_VIDEO_OUTPUT  0x00000002  /* Is a video output device */
#define V4L2_CAP_VIDEO_OVERLAY  0x00000004  /* Can do video overlay */
#define V4L2_CAP_VBI_CAPTURE  0x00000010  /* Is a raw VBI capture device */
#define V4L2_CAP_VBI_OUTPUT  0x00000020  /* Is a raw VBI output device */
#define V4L2_CAP_SLICED_VBI_CAPTURE 0x00000040  /* Is a sliced VBI capture device */
#define V4L2_CAP_SLICED_VBI_OUTPUT 0x00000080  /* Is a sliced VBI output device */
#define V4L2_CAP_RDS_CAPTURE  0x00000100  /* RDS data capture */
#define V4L2_CAP_VIDEO_OUTPUT_OVERLAY 0x00000200  /* Can do video output overlay */
#define V4L2_CAP_HW_FREQ_SEEK  0x00000400  /* Can do hardware frequency seek  */
#define V4L2_CAP_TUNER   0x00010000  /* has a tuner */
#define V4L2_CAP_AUDIO   0x00020000  /* has audio support */
#define V4L2_CAP_RADIO   0x00040000  /* is a radio device */
#define V4L2_CAP_READWRITE              0x01000000  /* read/write systemcalls */
#define V4L2_CAP_ASYNCIO                0x02000000  /* async I/O */
#define V4L2_CAP_STREAMING              0x04000000  /* streaming I/O ioctls */
	//设置视频格式
	tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	tv4l2_format.fmt.pix.width = 320; 
	tv4l2_format.fmt.pix.height = 240; 
	tv4l2_format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG; 
	tv4l2_format.fmt.pix.field = V4L2_FIELD_INTERLACED;
     if (ioctl(fd, VIDIOC_S_FMT, &tv4l2_format)< 0) {
         printf("VIDIOC_S_FMT\n");
         close(fd);
     }

	//请求V4L2驱动分配视频缓冲区（申请V4L2视频驱动分配内存）
	memset(&tV4L2_reqbuf, 0, sizeof(struct v4l2_requestbuffers ));
	tV4L2_reqbuf.count = 1;    //申请缓冲区的个数
	tV4L2_reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	tV4L2_reqbuf.memory = V4L2_MEMORY_USERPTR;
	tV4L2buf.length = jpgsize;
	tV4L2buf.m.userptr = (unsigned int)buffer;
     if (ioctl(fd, VIDIOC_REQBUFS, &tV4L2_reqbuf)< 0) {
         perror("VIDIOC_REQBUFS");
         close(fd);
     }
	//查询已经分配的V4L2的视频缓冲区的相关信息，包括视频缓冲区的使用状态、在内核空间的偏移地址、缓冲区长度等。在应用程序设计中通过调VIDIOC_QUERYBUF来获取内核空间的视频缓冲区信息，然后调用函数mmap把内核空间地址映射到用户空间，这样应用程序才能够访问位于内核空间的视频缓冲区。
	memset(&tV4L2buf, 0, sizeof(struct v4l2_buffer));
	tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	tV4L2buf.memory = V4L2_MEMORY_USERPTR; 
	tV4L2buf.length = jpgsize;
	tV4L2buf.m.userptr = (unsigned int)buffer;
	tV4L2buf.index = 0;  // 要获取内核视频缓冲区的信息编号
     if (ioctl(fd, VIDIOC_QUERYBUF, &tV4L2buf)< 0) {
         perror("VIDIOC_QUERYBUF");
         close(fd);
     }

// 把内核空间缓冲区映射到用户空间缓冲区
	//buffer = mmap(NULL,tV4L2buf.length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,tV4L2buf.m.offset);

	//投放一个空的视频缓冲区到视频缓冲区输入队列中
	memset(&tV4L2buf, 0, sizeof(struct v4l2_buffer));
	tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	tV4L2buf.memory = V4L2_MEMORY_USERPTR;
	tV4L2buf.length = jpgsize;
	tV4L2buf.m.userptr = (unsigned int)buffer;
	tV4L2buf.index = 0; //指令要投放到视频输入队列中的内核空间视频缓冲区的编号；

     if (ioctl(fd,VIDIOC_QBUF, &tV4L2buf)< 0) {
         perror("VIDIOC_QBUF");
         close(fd);
     }

	//启动视频采集命令，应用程序调用VIDIOC_STREAMON启动视频采集命令后，视频设备驱动程序开始采集视频数据，并把采集到的视频数据保存到视频驱动的视频缓冲区中
     if (ioctl(fd, VIDIOC_STREAMON, &v4l2type)< 0) {
         perror("VIDIOC_STREAMON");
         close(fd);
     }
	FD_ZERO(&fds); 
	FD_SET(fd,  &fds); 
	tv.tv_sec = 2;       // Timeout
	tv.tv_usec = 0; 
	select(fd+ 1, &fds, NULL, NULL, &tv);  
	//从视频缓冲区的输出队列中取得一个已经保存有一帧视频数据的视频缓冲区；
     if (ioctl(fd, VIDIOC_DQBUF, &tV4L2buf) < 0) {
         perror("VIDIOC_DQBUF");
         close(fd);
     }


     //for(i=0; i<2; i++)
        // read(fd, buffer, jpgsize);//从摄像头读取数据，连续读2次，这样能确保第1张图片是完整的，第2张丢弃

	//停止视频采集命令
     if (ioctl(fd, VIDIOC_STREAMOFF, &v4l2type)< 0) {
         perror("VIDIOC_STREAMOFF");
         close(fd);
     }

     i=0;p=buffer;
     while(!((*p==0xff)&&(*(p+1)==0xd9)))//计算jpg文件大小，jpg以0xffd9结尾
       {
        i++;
        p++;
       }
     jpgsize=i+2;

	printf("jpgsiez:%d\n",jpgsize);
	//printf("buffer:%s\n",buffer);

     if((fp1=fopen(JPG,"wb"))==NULL)//建立jpg文件
        error("Can   not   open   Jpg   File!");

     if((fp2=fopen(BMP,"wb"))==NULL)//建立bmp文件
        error("Can   not   create   Bmp   File!");

     fwrite(buffer,jpgsize,1,fp1);//数据写入jpg文件
     fclose(fp1);
     if((fp1=fopen(JPG,"rb"))==NULL)
        error("Can   not   open   Jpg   File!");
     jpg2bmp(fp1,fp2);//jpg转化为bmp并写入
     printf("get BMP form video\t[OK]\n");
	fclose(fp1);
	fclose(fp2);
	close(fd);
     return 0;

}
