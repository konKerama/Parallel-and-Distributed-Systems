%% NLM

clear all
close all

% Input img + dims
pathImg = '../data/house.mat';
strImgVar = 'house';

% noise
noiseParams = {'gaussian', 0, 0.001};

% filter sigma value
filtSigma = 0.02;
% patchSize = input('Patch [X Y] = ');
patchSize = [5 5];
patchSigma = 5/3;

%% USEFUL FUNCTIONS
% image normalizer
normImg = @(I) (I - min(I(:))) ./ max(I(:) - min(I(:)));

%% (BEGIN)
fprintf('...begin %s...\n',mfilename);

%% INPUT DATA
fprintf('...loading input data...\n')

ioImg = matfile( pathImg );
I     = ioImg.(strImgVar);

%% PREPROCESS
fprintf(' - normalizing image...\n')
I = normImg( I );
[m n] = size(I);

figure('Name','Original Image');
imagesc(I); axis image;
colormap gray;
saveas(gcf,'Results/originalImage.png');

% Pad matrix limits
padSize_X = (patchSize(1)-1)/2;
padSize_Y = (patchSize(2)-1)/2;

%% ADD MIRROR PADDING METHOD
% for i=1:padSize_X
%     A = [A A(:,size(A,2)+1-2*i)];
% end

% Pad column 0
for i=1:padSize_X
    I = [I(:,2*i) I];
end
% Pad column M
for i=1:padSize_X
    I = [I I(:,size(I,2)+1-2*i)];
end
    
% Pad row 0
I = I'; % Using transpose in order to recycle the previous sequence of instructions
for i=1:padSize_Y
    I = [I(:,2*i) I];
end
% Pad row N
for i=1:padSize_Y
    I = [I I(:,size(I,2)+1-2*i)];
end
I = I';

% New I Dims:
[mPadded nPadded] = size(I);
figure('Name','Padded Image');
imagesc(I); axis image;
colormap gray;
saveas(gcf,'Results/paddedImage.png');

%% NOISE
fprintf(' - applying noise...\n');
J = imnoise( I, noiseParams{:} );
figure('Name','Noisy-Input Image');
imagesc(J); axis image;
colormap gray;
saveas(gcf,'Results/noisyInputImage.png');
H = fspecial('gaussian',patchSize, patchSigma);

%% CUDA Kernel
threadsPerBlock = [16 16];
k = parallel.gpu.CUDAKernel('../cuda/NLMKernel.ptx','../cuda/NLMKernel.cu');
numberOfBlocks  = ceil( [mPadded nPadded] ./ threadsPerBlock );
k.ThreadBlockSize = threadsPerBlock;
k.GridSize        = numberOfBlocks;                                              
k.SharedMemorySize = 2*(threadsPerBlock(1)+2*padSize_X)*(threadsPerBlock(2) + 2*padSize_Y)*4;

imgBef = gpuArray(single(J));
imgAfter = gpuArray(zeros([mPadded nPadded] , 'single'));
startCuda = tic();
% Passing start/end indexes pre-padding for rows-columns e.g 64x64 with 5
% patchsize (padding=2) will start from 2 and end at 66
% m/nPadded will be equal to m/n+2*padding

startCuda = tic()
img = feval(k,imgBef, imgAfter, H, filtSigma,mPadded, nPadded, padSize_X);
 % wait(gpuDevice);
fprintf('[+] Time elapsed for Cuda execution: %.2f seconds\n', toc(startCuda));

gatherTic = tic();
If = gather(img);
fprintf('[+] Time elapsed for Gather: %.2f seconds\n', toc(gatherTic));

% Remove padding
% If = If(padSize_X + 1:m+padSize_X, padSize_Y+1:n+padSize_Y);

%% Visuals
figure('Name', 'Filtered image');
imagesc(If); axis image;
colormap gray;
saveas(gcf,'Results/FilteredImage.png');

% figure('Name', 'Residual');
% imagesc(If-J); axis image;
% colormap gray;
% saveas(gcf,'Results/ResidualImage.png');

%% (END)
fprintf('Done!\n');
