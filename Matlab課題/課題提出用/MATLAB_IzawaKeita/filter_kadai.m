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

%合成波形
y = sin(2*pi*5*t)+sin(2*pi*50*t)+sin(2*pi*80*t);
%合成波形描画
figure
subplot(4,2,1)
plot(t,y)
title('合成関数')
xlim([0 0.3])

%FFT変換
y_fft = fft(y);
power_y = abs(y_fft).^2/n;    %パワースペクトル
%FFTパワースペクトル描画
subplot(4,2,2)
plot(f,power_y)
title('FFTpower')


%バタワースフィルター
%ローパスフィルタ
[b_low,a_low] = butter(2,fc_low/(fs/2),'low');
y_low = filter(b_low,a_low,y);
%ローパスフィルタ波形
subplot(4,2,3)
plot(t,y_low)
title('2次ローパスフィルタ後')
xlim([0 0.3])
%FFT変換
y_low_fft = fft(y_low);
power_low = abs(y_low_fft).^2/n;    %パワースペクトル
%FFTパワースペクトル描画
subplot(4,2,4)
plot(f,power_low)
title('FFTpower')

%ハイパスフィルタ
[b_high,a_high] = butter(2,fc_high/(fs/2),'high');
y_high = filter(b_high,a_high,y);
%ハイパスフィルタ波形
subplot(4,2,5)
plot(t,y_high)
title('2次ハイパスフィルタ後')
xlim([0 0.3])
%FFT変換
y_high_fft = fft(y_high);
power_high = abs(y_high_fft).^2/n;    %パワースペクトル
%FFTパワースペクトル描画
subplot(4,2,6)
plot(f,power_high)
title('FFTpower')

%バンドパスフィルタ
[b_band,a_band] = butter(2,[fc_high fc_low]/(fs/2),'bandpass');
y_band = filter(b_band,a_band,y);
%バンドパスフィルタ波形
subplot(4,2,7)
plot(t,y_band)
title('2次バンドパスフィルタ後')
xlim([0 0.3])
%FFT変換
y_band_fft = fft(y_band);
power_band = abs(y_band_fft).^2/n;    %パワースペクトル
%FFTパワースペクトル描画
subplot(4,2,8)
plot(f,power_band)
title('FFTpower')

%png出力
print -dpng -r300 'filtter_kadai.png'

%フィルターの作成と適用(グラフも作成).
%フィルタの次数は2でやりました.
