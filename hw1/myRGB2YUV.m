function [yuv] = myRGB2YUV(rgb,Umax,Vmax)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

    length = size(rgb,1);
    width = size(rgb,2);
    yuv = zeros(length,width,3);
    rgb = double(rgb)/max(max(max(rgb)));

    wr = 0.299;
    wb = 0.114;
    wg = 1 - wr - wb;
  
    yuv(:,:,1) = wr*rgb(:,:,1) + wg*rgb(:,:,2) + wb*rgb(:,:,3);
    yuv(:,:,2) = Umax*(rgb(:,:,3) - yuv(:,:,1))/(1-wb);
    yuv(:,:,3) = Vmax*(rgb(:,:,1) - yuv(:,:,1))/(1-wr);
end
