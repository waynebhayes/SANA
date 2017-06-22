function NSORT(a,ai){delete sortTb;delete sortTc; for(i in a)sortTb[a[i]*(1+1e-7*rand())]=i;NsortTc=asorti(sortTb,sortTc);for(i=1;i<=NsortTc;i++)ai[i]=sortTb[sortTc[i]];return NsortTc}
#Bubble Sort: assumes 1-indexed arrays!
function bsort(array,outindices){
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

function ASSERT(cond,str){if(!cond){s=sprintf("assertion failure <%s>\n", str); print s; print s >"/dev/stderr"; exit 1}}
function ABS(x){return x<0?-x:x}
function SIGN(x){return x==0?0:x/ABS(x)}
function MAX(x,y){return x>y?x:y}
function MIN(x,y){return x<y?x:y}
function LSPredict(n, x, y, xIn) {
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
}
function StatAddSample(name, x) {
    if(1*_statN[name]==0)StatReset(name);
    _statN[name]++;
    _statSum[name]+=x;
    _statSum2[name]+=x*x;
    _statMin[name]=MIN(_statMin[name],x);
    _statMax[name]=MAX(_statMax[name],x);
}
function StatMean(name) {
    return _statSum[name]/_statN[name];
}
function StatVar(name) {
    return (_statSum2[name] - _statSum[name]*_statSum[name]/_statN[name]) / (_statN[name]-1);
}
function StatStdDev(name) {
    return sqrt(StatVar(name));
}
function StatN(name) {
    return _statN[name];
}
function NormalTail2X(quantile)
{
    q = quantile > 0.5 ? (1 - quantile) : quantile;
    z1 = sqrt (-2.0 * log (q));
    n = (0.010328 * z1 + 0.802853) * z1 + 2.515517;
    d = ((0.001308 * z1 + 0.189269) * z1 + 1.43278) * z1 + 1.0;
    z1 -= n / d;
    return (quantile > 0.5 ? -z1 : z1);
}
function NormalPhi(x)
{
    arg=-x*x/2;
    if(arg<-700) return 0;
    return 0.39894228040143267794*exp(arg)
}
function NormalX2Tail(x)
{
    if(x<0) return 1-NormalX2Tail(-x);
    b0 = 0.2316419; b1 = 0.319381530; b2 = -0.356563782; b3 = 1.781477937; b4 = -1.821255978; b5 = 1.330274429;
    t=1/(1+b0*x);
    paren = t*(b1+t*(b2+t*(b3+t*(b4+t*b5))));
    return NormalPhi(x)*paren;
}
function TDistTail2X (quantile, freedom)
{
    z1 = ABS(NormalTail2X(quantile));
    z2 = z1 * z1;
    h[0] = 0.25 * z1 * (z2 + 1.0);
    h[1] = 0.010416667 * z1 * ((5.0 * z2 + 16.0) * z2 + 3.0);
    h[2] = 0.002604167 * z1 * (((3.0 * z2 + 19.0) * z2 + 17.0) * z2 - 15.0);
    h[3] = z1 * ((((79.0 * z2 + 776.0) * z2 + 1482.0) * z2 - 1920.0) * z2 - 945.0);
    h[3] *= 0.000010851;

    x = 0.0;
    for (i = 3; i >= 0; i--)
    x = (x + h[i]) / freedom;
    z1 += x;
    return (quantile > 0.5 ? -z1 : z1);
}
function TDistX2Tail(t,n)
{
    if(t<0) return 1-TDistX2Tail(-t,n);
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
BEGIN{PI=M_PI=3.14159265358979324}
