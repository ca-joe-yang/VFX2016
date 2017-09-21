function [rgb] = myYUV2RGB(yuv,Umax,Vmax)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

    length = size(yuv,1);
    width = size(yuv,2);
    rgb = zeros(length,width,3);

    wr = 0.299;
    wb = 0.114;
    wg = 1 - wr - wb;
  
    rgb(:,:,1) = yuv(:,:,1) + yuv(:,:,3)*(1-wr)/Vmax;
    rgb(:,:,2) = yuv(:,:,1) - yuv(:,:,2)*wb*(1-wb)/(Umax*wg) - yuv(:,:,3)*wr*(1-wr)/(Vmax*wg);
    rgb(:,:,3) = yuv(:,:,1) + yuv(:,:,2)*(1-wb)/Umax; 
end
