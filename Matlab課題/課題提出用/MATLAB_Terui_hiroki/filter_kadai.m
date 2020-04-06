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

x1 = sin(2*pi*t*5);
x2 = sin(2*pi*t*50);
x3 = sin(2*pi*t*80);
x=x1+x2+x3;

subplot(3,3,1);
plot(t,x);
title('InputWave');
xlabel('time[s]');
ylabel('amplitude');
xlim([0 0.3]);
ylim([-2 2]);

X=abs(fft(x));

subplot(3,3,2);
plot(f,X);
title('InputWave(FFT)');
xlabel('Frequency[Hz]');
ylabel('amplitude');
%フィルターの作成と適用(グラフも作成).
%フィルタの次数は2でやりました.

[b,a] = butter(2,fc_high/(fs/2),'high');
HighPass = filter(b,a,x);
subplot(3,3,4);
plot(t,HighPass);

title('HighPass');
xlabel('time[s]');
ylabel('amplitude');
xlim([0 0.3]);
ylim([-2 2]);

[b,a] = butter(2,fc_low/(fs/2),'low');
LowPass = filter(b,a,x);
subplot(3,3,5);
plot(t,LowPass);
title('LowPass');
xlabel('time[s]');
ylabel('amplitude');

xlim([0 0.3]);
ylim([-2 2]);

[b,a] = butter(2,[fc_high fc_low]/(fs/2),'bandpass');
BandPass = filter(b,a,x);
subplot(3,3,6);
plot(t,BandPass);
title('BandPass');
xlabel('time[s]');
ylabel('amplitude');

xlim([0 0.3]);
ylim([-2 2]);

HIGHPASS = abs(fft(HighPass));
subplot(3,3,7);
plot(f,HIGHPASS);
title('HighPass(FFT)');
xlabel('Frequency[Hz]');
ylabel('amplitude');

LOWPASS = abs(fft(LowPass));
subplot(3,3,8);
plot(f,LOWPASS);
title('LowPass(FFT)');
xlabel('Frequency[Hz]');
ylabel('amplitude');

BANDPASS = abs(fft(BandPass));
subplot(3,3,9);
plot(f,BANDPASS);
title('BandPass(FFT)');
xlabel('Frequency[Hz]');
ylabel('amplitude');
