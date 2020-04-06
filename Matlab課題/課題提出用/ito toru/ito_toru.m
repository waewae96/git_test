clear;

n = 4096;
dt=0.005;
t=((1:n)-1)*dt;
f=t/dt/dt/n;    
fs=1/dt;
fc_high = 10;   %ハイパス用
fc_low = 70;    %ローパス用

%sin波の作成と合成波の作成
f1 = 5;
f2 = 50;
f3 = 80;
x1 = sin(2*pi*f1*t);
x2 = sin(2*pi*f2*t);
x3 = sin(2*pi*f3*t);
x=x1+x2+x3;

[b_l,a_l]=butter(2,2*fc_low/fs,'low');
[b_h,a_h]=butter(2,2*fc_high/fs,'high');

subplot(3,3,1)
plot(t,x)
xlim([0,0.3])
ylim([-2,2])

subplot(3,3,2)
y1 = fft(x,n);
y2 = abs(y1);
xaxis=linspace(0,200,n);
plot(xaxis,y2)
xlim([0,200])
ylim([0,2500])

%フィルターの作成と適用(グラフも作成).

subplot(3,3,4)
y_l=filter(b_l,a_l,x);
plot(t,y_l)
xlim([0,0.3])
ylim([-2,2])

subplot(3,3,5)
y_h=filter(b_h,a_h,x);
plot(t,y_h)
xlim([0,0.3])
ylim([-2,2])

subplot(3,3,6)
y_b1=filter(b_l,a_l,x);
y_b2=filter(b_h,a_h,y_b1);
plot(t,y_b2)
xlim([0,0.3])
ylim([-2,2])

subplot(3,3,7)
y1_l = fft(y_l,n);
y2_l = abs(y1_l);
xaxis=linspace(0,200,n);
plot(xaxis,y2_l)
xlim([0,200])
ylim([0,2500])

subplot(3,3,8)
y1_h = fft(y_h,n);
y2_h = abs(y1_h);
xaxis=linspace(0,200,n);
plot(xaxis,y2_h)
xlim([0,200])
ylim([0,2500])

subplot(3,3,9)
y1_b = fft(y_b2,n);
y2_b = abs(y1_b);
xaxis=linspace(0,200,n);
plot(xaxis,y2_b)
xlim([0,200])
ylim([0,2500])





