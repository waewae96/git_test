clear
dt=1/1000;       %サンプリング間隔
fs=1000;        %サンプリング周波数
fc_band_high = 100;   %バンドパス用
fc_band_low = 1.5;
fc_low = 1;

fileID = fopen('13_49_57check.txt', 'r');
%a = fscanf(fileID, '%c', 28);
sizeD = [5 Inf];
formatD = '%f %f %f %f %f';
d = fscanf(fileID, formatD, sizeD);
fclose(fileID);
size = numel(d)/5;
t=((1:size)-1)*dt;
f=t/dt/dt/size;    


fileID2 = fopen('13_49_57para.txt', 'r');
formatPara = '%f';
para = fscanf(fileID2, formatPara);
%p = strsplit(para);
fclose(fileID2);
figure(1)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (筋電データ)'])
end


[b_band,a_band] = butter(2, [fc_band_low/(fs/2) fc_band_high/(fs/2)], 'bandpass');
[b_low,a_low] = butter(2, fc_low/(fs/2), 'low');

%手順2　(index = 1は通し番号なので飛ばす)

for index = 2:5
    d(index, :) = filter(b_band, a_band, d(index, :));
end

figure(2)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (バンドパス)'])
end

%3
for index = 2:5
    d(index, :) = abs(d(index, :));
end

figure(3)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (全波整流)'])
end

%4
for index = 2:5;
    d(index, :) = filter(b_low, a_low, d(index,:));
end

figure(4)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (ローパス)'])
end

%5


for i = 2:5
  for j = 1:size
      if d(i, j) - para(i-1) < 0
          d(i, j) = 0;
      else
          d(i, j) = d(i, j) - para(i-1);
      end
  end
end

figure(5)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (オフセット除去)'])
end

%6
for index = 2:5
    d(index, :) = d(index, :) / para(i+3);
end

figure(6)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (正規化)'])
end

%7
for i = 1:size
    sum = 0;
    for ch = 2:5
        sum = sum + d(ch, i);
    end
    if sum ~= 0
        d(:, i) = d(:,i) / sum;
    end
end

figure(7)
for index = 1:4
    ylim([-1 1])
    subplot(2, 2, index)
    plot(t, d(index+1, :));
    title(['Ch', num2str(index), ' (総和1)'])
end
