%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

% Reading the files

function [Y] = myImageVariance(X, w) 

    % X(P,i,j,3)
    s = size(X);
    P = s(1);
    length = s(2);
    width = s(3);
    w=(w+1)/256;
    wtotal = zeros(P,3);
    for n = 1:P
        for ch = 1:3
            for i = 1:length
                for j = 1:width
                    wtotal(n,ch) = wtotal(n,ch) + w(n,i,j,ch);
                end
            end
        end
    end
    wtotal
    avg = zeros(length,width,s(4));
    X = w.*X;
 
    for n = 1:P
        for ch = 1:3
            avg(:,:,ch) = avg(:,:,ch) + squeeze(X(n,:,:,ch));
        end
    end
    avg = avg/P;
    
    Y = zeros(length,width,s(4));
    for n = 1:P
        for ch = 1:3
            Y(:,:,ch) = Y(:,:,ch) + (squeeze(X(n,:,:,ch))-avg(:,:,ch)).^2;
        end
    end
    Y = Y/P;
    Y = realsqrt(Y);
    Y = Y/256;
    figure;
    imshow(Y);
    axis image;
  end
