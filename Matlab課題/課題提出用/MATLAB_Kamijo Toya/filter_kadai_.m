clear
n=4096;         %データ数
dt=0.005;       %サンプリング間隔
t=((1:n)-1)*dt;
f=t/dt/dt/n;    
fs=1/dt;        %サンプリング周波数
fc_high = 10;   %ハイパス用
fc_low = 70;    %ローパス用

%遮断周波数やサンプル数などは上のものを使ってください

%sin波の作成と合成波の作成

w1=10*pi;       
w2=100*pi;
w3=160*pi;

s1=sin(w1*t);
s2=sin(w2*t);
s3=sin(w3*t);

g=s1+s2+s3; %合成関数
Fg=fft(g,n);
aFg=abs(Fg);            %絶対値

subplot(3,3,1)
plot(t,g)               %g(t)の図示
xlim([0 0.3])
xlabel('time [s]');
ylim([-2 2])
ylabel('g(t)'); 

subplot(3,3,2)
plot(f,aFg)             %FFT[g(t)]の図示
xlim([0 200])
xlabel('freaquency [Hz]');
ylim([0 2500])
ylabel('FFT[g(t)]');


%フィルターの作成と適用(グラフも作成).
%フィルタの次数は2でやりました.


%ローパスフィルタ

[b,a] = butter(2,fc_low/(fs/2),'low');
%freqz(b,a)
y1=filter(b,a,g);
Fy1=fft(y1,n);
aFy1=abs(Fy1);

subplot(3,3,4)
plot(t,y1)
xlim([0 0.3])
ylim([-2 2])

subplot(3,3,7)
plot(f,aFy1)             %FFT[g(t)]の図示
xlim([0 200])
xlabel('freaquency [Hz]');
ylim([0 2000])
ylabel('FFT[g(t)]');

%ハイパスフィルタ

[b,a] = butter(2,fc_high/(fs/2),'high');
%freqz(b,a)
y2=filter(b,a,g);
Fy2=fft(y2,n);
aFy2=abs(Fy2);

subplot(3,3,5)
plot(t,y2)
xlim([0 0.3])
ylim([-2 2])

subplot(3,3,8)
plot(f,aFy2)             %FFT[g(t)]の図示
xlim([0 200])
xlabel('freaquency [Hz]');
ylim([0 2000])
ylabel('FFT[g(t)]');

%バンドパスフィルタ

[b,a] = butter(2,[fc_high fc_low]/(fs/2),'bandpass');
%freqz(b,a)
y3=filter(b,a,g);
Fy3=fft(y3,n);
aFy3=abs(Fy3);

subplot(3,3,6)
plot(t,y3)
xlim([0 0.3])
ylim([-2 2])

subplot(3,3,9)
plot(f,aFy3)             %FFT[g(t)]の図示
xlim([0 200])
xlabel('freaquency [Hz]');
ylim([0 2000])
ylabel('FFT[g(t)]');
