%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

function [hdr] = myHDR(imagesDir, format, algorithm, t)

    path = pwd;
    cd(imagesDir); % go to the folder
    currentFile = dir ; % read folder's content
    images = {} ;

    % Reading the files
    for i = 1:numel(currentFile) % look for files
        if ~currentFile(i).isdir % discard subfolders
            if strcmpi(currentFile(i).name(end-3:end), format)
                images{end+1} = currentFile(i).name ; %filename
            end
        end
    end

    P = numel(images);
    % Reading the images
    X0 = imread(images{1});
    length = size(X0,1);
    width = size(X0,2);
    X = zeros(P, length, width, 3);
    X (1,:,:,:) = X0;
    for j = 1:P
        X(j,:,:,:) = imread(images{j});
    end
    cd (path);

    % implement MTB alignment
    X = myMTB(X);
    % Ghost
    Y = myGhost(X, 100);
    % Randomly choose N points
    N = 50;
    NXY = rand(N,2);
    NXY(:,1) = (length-1)*NXY(:,1)+1;
    NXY(:,2) = (width-1)*NXY(:,2)+1;

    % generating Z(i,j)
    Z = zeros(N,P,3);
    for j = 1:P
        for i = 1:N
            x = round(NXY(i,1));
            y = round(NXY(i,2));
            Z(i,j,:) = X(j,x,y,:);
        end
    end
    
    % Constructing weight function --- a Triangle
    w = cat(2,linspace(0.01,1,128),linspace(1,0.01,128));
    w = w/sum(w);

    hdr = zeros(length,width,3);
    if algorithm == 1
        for ch = 1:3
            B = log10(t);
            Zch = squeeze(Z(:,:,ch));
            [g,lE] = gsolve(Zch,B,50,w);

            figure;
            plot(0:255, g);
        
            for x = 1:length
                for y = 1:width
                    a = 0;
                    b = 0;
                    for j = 1:P
                        z = X(j,x,y,ch);
                        a = a + w(z + 1)*(g(z + 1)-B(j));
                        b = b + w(z + 1);
                    end
                    E = power(10, a/b);
                    hdr(x,y,ch) = E;
                end
            end
            figure;
            fig = surface(hdr(:,:,ch));
            axis image;
            colormap jet;
            set(fig,'LineStyle','none')
            colorbar;
        end
    elseif algorithm == 2
        M = 10;
        Z = (double(Z)+1)/256;
        for ch = 1:3
            Zch = squeeze(Z(:,:,ch));
            g = gsolve2(Zch,M);
            for x = 1:length
                for y = 1:width
                    a = 0;
                    b = 0;
                    for j = 1:P
                        z = X(j,x,y,ch);
                        a = a + w(z+1)*g(z+1);
                        b = b + g(z+1);
                    end
                    E = a/b;
                    %validateattributes(E,{'double'},{'finite'})
                    hdr(x,y,ch) = E;
                end
            end
            figure;
            fig = surface(hdr(:,:,ch));
            axis image;
            colormap jet;
            set(fig,'LineStyle','none')
            colorbar;
        end
    end
end
