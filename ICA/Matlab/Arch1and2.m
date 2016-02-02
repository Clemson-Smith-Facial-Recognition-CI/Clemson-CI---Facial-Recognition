% script Arch1.m
% Finds ICA representation of train and test images under Architecture I, 
% described in Bartlett & Sejnowski (1997, 1998), and Bartlett, Movellan & 
% Sejnowski (2002):  In Architecture I, we load N principal component 
% eigenvectors into rows of x, and then run ICA on x.
%
% Put the aligned training images in the rows of C, one image per row.  
% In the following examples, there are 500 images of aligned faces of size 
% 60x60 pixels, so C is 500x3600. 
%
% You can use the following matlab code to create C:
% markFeatures.m collects eye and mouth positions. 
% align_Faces.m crops, aligns, and scales the face images.
% loadFaceMat.m loads the images into the rows of C. 
%
% This script also calls the matrix of PCA eigenvectors organized in 
% the columns of V (3600x499), created by [V,R,E] = pcabigFn(C');
%
% The ICA representation will be in the rows of F (called B in Bartlett, 
% Movellan & Sejnowski, 2002): 

loadTestMat;
loadFaceMat;

[V,R,E] = pcabigFn(C');
%D = zeroMn(C')'; % D is 500x3600 and D = C-ones(500,1)*mean(C);
%R = D*V; 	 % R is 500x499 and contains the PCA coefficients;

% We choose to use the first 200 eigenvectors. 
% (If PCA generalizes better by dropping first few eigenvectors, ICA will too).

x = V(:,1:116)'; 		% x is 200x3600
runica 				% calculates wz, w and uu. The matrix x gets 
				% overwritten by a sphered version of x. 
F1 = R(:,1:116) * inv(w*wz); 	% F is 500x200 and each row contains the 
			    	% ICA1 rep of an image
F2 = uu'; 		% F is 500x200 and each row contains the ICA2 rep of 1 image. 
			% F = w * wz * zeroMn(R(:,1:200)')'; is the same thing.

% Representations of test images under architecture I: 
% Put original aligned test images in rows of Ctest. 
Ctest = [];
[FName, PName, FIndex] = uigetfile();
I = imread(strcat(PName, FName));
tmp = mat2gray(double(I));
tmp = reshape(tmp,1,size(tmp,1)*size(tmp,2));
Ctest = [Ctest;tmp];
Dtest = zeroMn(Ctest')'; % For proper testing, subtract the mean of the 
			 % training images not the test images: 
			 % Dtest = Ctest-ones(500,1)*mean(C);
Rtest = Dtest*V;
F1test = Rtest(:,1:116) * inv(w*wz);
F2test = w * wz * zeroMn(Rtest(:,1:116)');

% Test nearest neighbor classification using cosine, not euclidean distance, 
% as similarity measure.
%
% First create label vectors. These are column vectors of integers. Lets 
% say our 500 training examples consisted of 500 different people. Then
trainClass = [1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8 9 9 9 9 10 10 10 10 11 11 11 11 12 12 12 12 13 13 13 13 14 14 14 14 15 15 15 15 16 16 16 16 17 17 17 17 18 18 18 18 19 19 19 19 20 20 20 20 21 21 21 21 22 22 22 22 23 23 23 23 24 24 24 24 25 25 25 25 26 26 26 26 27 27 27 27 28 28 28 28 29 29 29 29]'; 
%
% We also need the correct class labels of the test examples if we want to 
% compute percent correct. Lets say the test examples were two images each 
% of the first 10 individuals. Then 
testClass = [1]';

%We now compute percent correct:
train_ex1 = F1';
test_ex1 = F1test';
fprintf(1,'ICA1 results')
[pc1,rankmat1] = nnclassFn(train_ex1,test_ex1,trainClass,testClass);
train_ex2 = F2';
test_ex2 = F2test;
fprintf(1,'ICA2 results')
[pc2,rankmat2] = nnclassFn(train_ex2,test_ex2,trainClass,testClass);
%pc is percent correct of first nearest neighbor.
%rankmat gives the top 30 matches for each test image. 

