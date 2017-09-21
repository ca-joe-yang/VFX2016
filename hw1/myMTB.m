function [X] = myMTB(X)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
    
    P = size(X,1);
    length = size(X,2);
    width = size(X,3);
   
    Y = zeros(P,length,width);
    % Median Threshold Bitmap
    for n = 1:P
        A = rgb2gray(squeeze(X(n,:,:,:)));
        B = reshape(A',1,length*width);
        m = median(B);
        for i = 1:length
            for j = 1:width
                if A(i,j) >= m 
                    Y(n,i,j) = 255;
                else 
                    Y(n,i,j) = 0;
                end
            end
        end
    end
  
    M = 5;
    moves = zeros(P,M+1,2);
    P2 = uint8(P/2);
    for m = 1:M+1 
        r = 2.^(M+1-m);
        Z0 = imresize(squeeze(Y(P2,:,:)), 1/r);
        for n = 1:P
            if n == P2
                continue
            end
            Z = imresize(squeeze(Y(n,:,:)), 1/r);
            diff = sum(sum((Z-Z0).^2));
            for x = -1:1
                for y = -1:1
                    Zxy = imtranslate(Z,[x,y]); 
                    d = (Zxy-Z0).^2;
                    d = sum(sum(d));
                    if d < diff
                        diff = d;
                        moves(n,m,:) = [x,y]*r;
                    end
                end
            end
            Y(n,:,:) = imtranslate(squeeze(Y(n,:,:)), squeeze(moves(n,m,:)));
        end
    end

    s = sum(moves,2);
    for n = 1:P
        if n == P2
            continue
        end
        X(n,:,:,:) = imtranslate(squeeze(X(n,:,:,:)), squeeze(s(n,:)));
    end
end
