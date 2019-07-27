BEGIN{PI=M_PI=3.14159265358979324}
function int2binary(i){if(i<=0)return "0";_s="";while(i){_s=(i%2)""_s;i=int(i/2)}return _s}
function Fatal(msg){printf "FATAL ERROR: %s\n",msg >"/dev/fd/2"; exit(1);}
function NormDotProd(u,v,    _dot,_dot1,_dot2,i){_dot=_dot1=_dot2=0;
    for(i in u){_dot+=u[i]*v[i];_dot1+=u[i]*u[i];_dot2+=v[i]*v[i]};
    return _dot/sqrt(_dot1*_dot2);
}
function inarray(element,array,      i){for(i=1;i<=length(array);i++)if(element==array[i])return 1; return 0}
function IsPrime(N,   i){for(i=2;i<=sqrt(N); i++)if(N/i==int(N/i))return 0; return 1}
function NSORT(a,ai,   i,NsortTc){delete sortTb;delete sortTc; for(i in a)sortTb[a[i]*(1+1e-7*rand())]=i;NsortTc=asorti(sortTb,sortTc);for(i=1;i<=NsortTc;i++)ai[i]=sortTb[sortTc[i]];return NsortTc}
#Bubble Sort: assumes 1-indexed arrays!
function bsort(array,outindices,    N,i,j,temp){
    delete outindices;delete iSortValue;
    N=0; for(i in array){N++;outindices[N]=i;iSortValue[N]=array[i]}
    for(i=2;i<=N;i++)
    {
	# Invariant: array from 1 .. i-1 is sorted, now bubble the next element into its place
	j=i;
	while(j>1 && iSortValue[j] < iSortValue[j-1]) {
	    temp=iSortValue[j-1];iSortValue[j-1]=iSortValue[j];iSortValue[j]=temp;
	    temp=outindices[j-1];outindices[j-1]=outindices[j];outindices[j]=temp;
	    j--;
	}
    }
    return N;
}
function asin(x) { return atan2(x, sqrt(1-x*x)) }
function acos(x) { return atan2(sqrt(1-x*x), x) }
function atan(x) { return atan2(x,1) }
function sind(x) { return sin(x/180*PI) }
function cosd(x) { return cos(x/180*PI) }
function tand(x) { return tan(x/180*PI) }
function asind(x) { return asin(x)/PI*180 }
function acosd(x) { return acos(x)/PI*180 }
function atand(x) { return atan(x)/PI*180 }
function fact(k) {if(k<=0)return 1; else return k*fact(k-1);}
function choose(n,k,     r,i) {r=1;for(i=1;i<=k;i++)r*=(n-(k-i))/i; return r}
function NumBits(n,    b) {b=0;while(n>0){if(n%2==1)b++;n=int(n/2)}; return b}

# And now counting the info in an edge list. One way to view the info is simply the number of edges.
# Another is to view each node's adjacency list as having log(n) bits for each of its neighbors.
# This then says that the amount of info is as follows: the end of each edge is listed twice (ie is in two neighbor lists),
# but only one is strictly needed. And each entry is log2(n) bits. So the total info is just log2(n)*numEdges.
# But this number is *way* bigger than the number of edges for all 2018 BioGRID networks, so clearly it's too high.
function netbits(n1,n2,     i,bits){if(n2==0)n2=n1; bits=0;for(i=0;i<MIN(n1,n2);i++)bits+=log(MAX(n1,n2)-i)/log(2); return bits}

function logb(b,x){return log(x)/log(b)}
function dtob(n,   s,sgn) {n=1*n;if(!n)return "0";s=sgn="";if(n<0){sgn="-";n=-n};while(n){s=sprintf("%d%s",(n%2),s); n=int(n/2)}; return sgn s}
function btod(n) {}

function ASSERT(cond,str){if(!cond){s=sprintf("assertion failure: %s\n", str); print s; print s >"/dev/stderr"; exit 1}}
function ABS(x){return x<0?-x:x}
function SIGN(x){return x==0?0:x/ABS(x)}
function MAX(x,y){return x>y?x:y}
function MIN(x,y){return x<y?x:y}
function LSPredict(n, x, y, xIn,      SUMx,SUMy,SUMxy,SUMxx,i,slope,y_intercept,x_intercept) {
    SUMx=SUMy=SUMxy=SUMxx=0;
    for(i=0;i<n;i++)
    {
	SUMx += x[i];
	SUMy += y[i];
	SUMxy += x[i]*y[i];
	SUMxx += x[i]*x[i];
    }
    if(n>0 && (SUMx*SUMx - n*SUMxx) != 0) {
	slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
	y_intercept = ( SUMy - slope*SUMx ) / n;
	x_intercept = 1e30;
	if(slope != 0) x_intercept = -y_intercept / slope;
	return slope*xIn + y_intercept;
    }
}
function StatReset(name) {
    _statN[name] = _statSum[name] = _statSum2[name] = 0;
    _statMin[name]=1e30;_statMax[name]=-1e30;
    _statmin[name]=1e30
}
function StatAddSample(name, x) {
    if(1*_statN[name]==0)StatReset(name);
    _statN[name]++;
    _statSum[name]+=x;
    _statSum2[name]+=x*x;
    _statMin[name]=MIN(_statMin[name],x);
    if(x)_statmin[name]=MIN(_statmin[name],x);
    _statMax[name]=MAX(_statMax[name],x);
}
function StatMean(name) {
    return _statSum[name]/_statN[name];
}
function StatMin(name) {
    return _statMin[name];
}
function Statmin(name) {
    return _statmin[name];
}
function StatMax(name) {
    return _statMax[name];
}
function StatVar(name) {
    return (_statSum2[name] - _statSum[name]*_statSum[name]/_statN[name]) / (_statN[name]-1);
}
function StatStdDev(name,     x) {
    x=StatVar(name);if(x<0)return 0;
    return sqrt(StatVar(name));
}
function StatN(name) {
    return _statN[name];
}
function Norm2(n,vec,      i,sum2)
{
    sum2 = 0;
    for(i=0;i<n;i++)sum2+=vec[i]*vec[i];
    return sqrt(sum2);
}
function NormalPtoZ(quantile,    q,z1,n,d)
{
    #printf "NormalPtoZ input is %g\n", quantile
    q = quantile > 0.5 ? (1 - quantile) : quantile;
    #printf "NormalPtoZ q is %g\n", q
    z1 = sqrt (-2.0 * log (q));
    n = (0.010328 * z1 + 0.802853) * z1 + 2.515517;
    d = ((0.001308 * z1 + 0.189269) * z1 + 1.43278) * z1 + 1.0;
    z1 -= n / d;
    return (quantile > 0.5 ? -z1 : z1);
}
function NormalPhi(x,    arg)
{
    arg=-x*x/2;
    if(arg<-700) return 0;
    return 0.39894228040143267794*exp(arg)
}
function NormalDist(mu,sigma,x){E=(mu-x)^2/(2*sigma*sigma);return exp(-E)/(sigma*2.506628274631000502415765284811)}
function NormalZ2P(x,    b0,b1,b2,b3,b4,b5,t,paren)
{
    if(x<0) return 1-NormalZ2P(-x);
    b0 = 0.2316419; b1 = 0.319381530; b2 = -0.356563782; b3 = 1.781477937; b4 = -1.821255978; b5 = 1.330274429;
    t=1/(1+b0*x);
    paren = t*(b1+t*(b2+t*(b3+t*(b4+t*b5))));
    return NormalPhi(x)*paren;
}
function StatTDistPtoZ(quantile, freedom,        z1,z2,h,x,i)
{
    #printf "StatTDistPtoZ input is (%g,%d)\n",quantile,freedom;
    z1 = ABS(NormalPtoZ(quantile));
    z2 = z1 * z1;
    h[0] = 0.25 * z1 * (z2 + 1.0);
    h[1] = 0.010416667 * z1 * ((5.0 * z2 + 16.0) * z2 + 3.0);
    h[2] = 0.002604167 * z1 * (((3.0 * z2 + 19.0) * z2 + 17.0) * z2 - 15.0);
    h[3] = z1 * ((((79.0 * z2 + 776.0) * z2 + 1482.0) * z2 - 1920.0) * z2 - 945.0);
    h[3] *= 0.000010851;

    x = 0.0;
    for(i = 3; i >= 0; i--)
    x = (x + h[i]) / freedom;
    z1 += x;
    return (quantile > 0.5 ? -z1 : z1);
}
function StatTDistZtoP(t,n,     z,y,b,a)
{
    if(t<0) return 1-StatTDistZtoP(-t,n);
    z=1; t=t*t;y=t/n;b=1+y;
    if(n>=20&&t<n||n>200){
	if(y>1e-6)y=log(b);
	a=n-.5;b=48*a*a;y=a*y;
	y=(((((-.4*y-3.3)*y-24)*y-85.5)/(.8*y*y+100+b)+y+3)/b+1)*sqrt(y);
	return NormalPhi(-y);
    } else {
	a=y=sqrt(y);if(n==1)a=0;
	n-=2;
	while(n>1) {
	    a=a*(n-1)/(b*n)+y;
	    n=-2;
	}
	a=(n==0?a/sqrt(b):(atan2(y,1)+a/b)*0.63661977236);
	return ABS(z-a)/2;
    }
}
function StatConfidenceInterval(name,conf)
{
    return StatTDistPtoZ((1-conf)/2, _statN[name] - 1) * sqrt(StatVar(name) / _statN[name]);
}
function Pearson2T(n,r){if(r==1)return 1e30; else return r*sqrt((n-2)/(1-r^2))}
function PoissonDistribution(l,k){if(l>700)return NormalDist(l,sqrt(l),k);r=exp(-l);for(i=k;i>0;i--)r*=l/i;return r} 
function PoissonCDF(k, lambda, sum, term, i){sum=term=1;for(i=1;i<=k;i++){term*=lambda/i;sum+=term}; return sum*exp(-lambda)}
function StatRV_Normal(){if(!_StatRV_which) { do { _StatRV_v1 = 2*rand()-1; _StatRV_v2 = 2*rand()-1; _StatRV_rsq = _StatRV_v1^2+_StatRV_v2^2; } while(_StatRV_rsq >= 1 || _StatRV_rsq == 0); _StatRV_fac=sqrt(-2*log(_StatRV_rsq)/_StatRV_rsq); _StatRV_next = _StatRV_v1*_StatRV_fac; _StatRV_which = 1; return _StatRV_v2*_StatRV_fac; } else { _StatRV_which = 0; return _StatRV_next; } } 
function PearsonAddSample(name,X,Y) {
    _Pearson_sumX[name]+=X
    _Pearson_sumY[name]+=Y
    _Pearson_sumXY[name]+=X*Y
    _Pearson_sumX2[name]+=X*X
    _Pearson_sumY2[name]+=Y*Y
    _Pearson_NR[name]++;
}
function PearsonCompute(name,     numer,D1,D2,denom){
    if(!_Pearson_NR[name])return;
    numer=_Pearson_sumXY[name]-_Pearson_sumX[name]*_Pearson_sumY[name]/_Pearson_NR[name]
    D1=_Pearson_sumX2[name]-_Pearson_sumX[name]*_Pearson_sumX[name]/_Pearson_NR[name]
    D2=_Pearson_sumY2[name]-_Pearson_sumY[name]*_Pearson_sumY[name]/_Pearson_NR[name]
    denom=sqrt(D1*D2); _Pearson_rho[name]=0; if(denom)_Pearson_rho[name]=numer/denom;
    _Pearson_t[name]=Pearson2T(_Pearson_NR[name],_Pearson_rho[name]);
    if(_Pearson_t[name]<0)_Pearson_t[name]=-_Pearson_t[name];
    _Pearson_p[name]=StatTDistZtoP(_Pearson_t[name],_Pearson_NR[name]-2)/1.23
}
function PearsonPrint(name){
    #if(!_Pearson_NR[name])return;
    PearsonCompute(name);
    # NR = number of samples, rho=Pearson correlation, p=p-value, t = number of standard deviations from random.
    return sprintf("%d %.3g %.3g %.3g", _Pearson_NR[name], _Pearson_rho[name], _Pearson_p[name], _Pearson_t[name])
}

# Functions for computing the AUPR
function AUPR_add(name, value, thresh, truth){
    _AUPR_N[name]++;
    if(value > thresh){ # predicted
	if(truth)++_AUPR_TP[name];else ++_AUPR_FP[name]
    }else{ # not predicted
	if(truth)++_AUPR_FN[name];else ++_AUPR_TN[name]
    }
}
function AUPR_Prec(name,  TP, FP){TP=_AUPR_TP[name];FP=_AUPR_FP[name];if(TP+FP==0)return 1; else return TP/(TP+FP)}
function AUPR_Rec(name,     TP, FN){TP=_AUPR_TP[name];FN=_AUPR_FN[name];if(TP+FN==0)return 1; else return TP/(TP+FN)}
function AUPR_F1(name,         Prec,Rec){Prec=AUPR_Prec(name); Rec=AUPR_Rec(name);
    if(Prec+Rec==0)return 0; else return 2*Prec*Rec/(Prec+Rec)
}
function AUPR_TPR(name,  TP, FN){TP=_AUPR_TP[name];FN=_AUPR_FN[name]; return TP/(TP+FN)}
function AUPR_FPR(name,  FP, TN){FP=_AUPR_FP[name];TN=_AUPR_TN[name]; return FP/(FP+TN)}

