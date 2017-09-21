%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

% Reading the files

function [Y] = myGhost(X, threshold) 

    P =size(X,1);
    length = size(X,2);
    width = size(X,3);
    XE = zeros(P,length,width,3);
    for n = 1:P
        XE(n,:,:,:) = mySobelEdge(squeeze(X(n,:,:,:)), 100);
    end
    
    Y = myImageVariance(X,XE);
    Y = im2bw(Y,0.1);
    figure;
    se = strel('square',1);
    Ydi = imdilate(Y,se);
    imshow(Ydi);
    axis image;

end
