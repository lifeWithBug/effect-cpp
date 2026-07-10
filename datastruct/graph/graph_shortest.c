#include <stdio.h>
#include <limits.h>

#define MAXV 20
#define INF INT_MAX/2

void dijkstra(int g[MAXV][MAXV], int n, int src) {
    int dist[MAXV], prev[MAXV], vis[MAXV]={0};
    for (int i=0;i<n;i++){dist[i]=g[src][i];prev[i]=src;}
    dist[src]=0;vis[src]=1;

    for (int step=0;step<n-1;step++){
        int u=-1,md=INF;
        for(int i=0;i<n;i++)if(!vis[i]&&dist[i]<md){md=dist[i];u=i;}
        if(u==-1)break;
        vis[u]=1;
        for(int v=0;v<n;v++){
            if(!vis[v]&&g[u][v]!=INF){
                int nd=dist[u]+g[u][v];
                if(nd<dist[v]){dist[v]=nd;prev[v]=u;}
            }
        }
    }

    printf("Dijkstra(源点%d):\n",src);
    for(int i=0;i<n;i++){
        printf("  %d->%d:%2d 路径:",src,i,dist[i]);
        int p[MAXV],len=0,cur=i;
        while(cur!=src){p[len++]=cur;cur=prev[cur];}
        printf("%d",src);
        for(int j=len-1;j>=0;j--)printf("->%d",p[j]);
        printf("\n");
    }
    printf("\n");
}

void floyd(int g[MAXV][MAXV],int n){
    int d[MAXV][MAXV],nx[MAXV][MAXV];
    for(int i=0;i<n;i++)for(int j=0;j<n;j++){d[i][j]=g[i][j];nx[i][j]=(g[i][j]!=INF)?j:-1;}
    for(int k=0;k<n;k++)
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                if(d[i][k]+d[k][j]<d[i][j]){d[i][j]=d[i][k]+d[k][j];nx[i][j]=nx[i][k];}

    printf("Floyd 距离矩阵:\n");
    printf("   ");
    for(int i=0;i<n;i++)printf("%3d",i);
    printf("\n");
    for(int i=0;i<n;i++){printf("%d: ",i);for(int j=0;j<n;j++)printf("%3d",d[i][j]);printf("\n");}
    printf("\n");

    printf("Floyd 路径(0到各顶点):\n");
    for(int j=0;j<n;j++){
        printf("  0->%d:%2d 路径:0",j,d[0][j]);
        int c=0;
        while(c!=j){c=nx[c][j];printf("->%d",c);}
        printf("\n");
    }
    printf("\n");
}

int main(){
    int n=4,g[MAXV][MAXV];
    for(int i=0;i<n;i++)for(int j=0;j<n;j++)g[i][j]=(i==j)?0:INF;

    /*    0 ---2--- 1
     *    |         |
     *    1         3
     *    |         |
     *    3 ---1--- 2
     */
    g[0][1]=g[1][0]=2; g[0][3]=g[3][0]=1;
    g[1][2]=g[2][1]=3; g[2][3]=g[3][2]=1;

    dijkstra(g,n,0);
    floyd(g,n);

    printf("预期: 0->1=2, 0->2=2(0-3-2), 0->3=1\n");
    return 0;
}
