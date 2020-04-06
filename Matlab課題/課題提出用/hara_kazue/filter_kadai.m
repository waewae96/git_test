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

%x = (0:dt:dt*(n-1));
y1 = sin(2*pi*5*t);
y2 = sin(2*pi*50*t);
y3 = sin(2*pi*80*t);
y = y1 + y2 + y3;

figure(1)
subplot(4,2,1);
plot(t,y)
title('合成波形')
xlim([0 0.3])

ft = fft(y);
m = abs(ft);
x2 = (0:fs/n:(length(m)-1)*fs/n);
subplot(4,2,2);
plot(x2,m)
title('FFT')

%フィルターの作成と適用(グラフも作成).
%フィルタの次数は2でやりました.

[b_low,a_low] = butter(2, fc_low/(fs/2), 'low');
[b_high,a_high] = butter(2, fc_high/(fs/2), 'high');
[b_band,a_band] = butter(1, [fc_high/(fs/2) fc_low/(fs/2)], 'bandpass');

y_low = filter(b_low, a_low, y);
y_high = filter(b_high, a_high, y);
y_band = filter(b_band, a_band, y);
subplot(4,2,3);
plot(t,y_low)
title('ローパスフィルタ')
xlim([0 0.3])
subplot(4,2,5);
plot(t,y_high)
title('ハイパスフィルタ')
xlim([0 0.3])
subplot(4,2,7);
plot(t,y_band)
title('バンドパスフィルタ')
xlim([0 0.3])

l = fft(y_low);
h = fft(y_high);
b = fft(y_band);
low = abs(l);
high = abs(h);
band = abs(b);
subplot(4,2,4);
plot(x2,low);
title('FFT')
subplot(4,2,6);
plot(x2,high)
title('FFT')
subplot(4,2,8);
plot(x2,band)
title('FFT')

% ダウンサンプリング（サンプル数を減らす）
figure(2)
subplot(3,1,1);
plot(t,y)
title('合成波形')
xlim([0 0.3])
y_down = downsample(y, 5);
x_down = downsample(t, 5);
subplot(3,1,2)
plot(x_down,y_down)
title('ダウンサンプリング')
xlim([0 0.3])

% アップサンプリング：3次スプライン補間（サンプル数を増やす）
x_up = (0:dt/5:dt*(n-1));
y_up = spline(t,y,x_up);
subplot(3,1,3)
plot(x_up,y_up)
title('アップサンプリング')
xlim([0 0.3])