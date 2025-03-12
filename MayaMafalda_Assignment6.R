#install.packages("tidyverse")
#install.packages("leaps")
#install.packages("ISLR") 

library(tidyverse)
library(leaps)
library(ISLR)

data <- Hitters
str(data)
data<-na.omit(data)

# Goal: We are going to use different methods to select a linear regression
# model for explaining the observed variation in salary across players.

data$League<-NULL
data$Division<-NULL
data$NewLeague<-NULL

data$Salary<-log(data$Salary)

# PART I: Best subset method
best_subset <- regsubsets(Salary ~ ., data = data, nvmax = 16)
summary(best_subset)

results<-summary(best_subset)
results$adjr2
results$bic

# a)

# Plot Adjusted R^2
plot(results$adjr2, type = "b", xlab = "Number of Variables", ylab = "Adjusted R^2", 
     main = "Adjusted R^2 vs. Number of Variables")
points(which.max(results$adjr2), max(results$adjr2), col = "red", cex = 2, pch = 20)

# Plot BIC
plot(results$bic, type = "b", xlab = "Number of Variables", ylab = "BIC", 
     main = "BIC vs. Number of Variables")
points(which.min(results$bic), min(results$bic), col = "red", cex = 2, pch = 20)

# Optimal model size based on adjusted R^2
optimal_adjr2_size <- which.max(results$adjr2)
cat("Optimal model size based on adjusted R^2:", optimal_adjr2_size, "\n")

# Optimal model size based on BIC
optimal_bic_size <- which.min(results$bic)
cat("Optimal model size based on BIC:", optimal_bic_size, "\n")

# b)

# 5. Extract Coefficients for Optimal Models**:
# Coefficients of the model with the highest adjusted R^2
best_adjr2_model <- coef(best_subset, optimal_adjr2_size)
print("Coefficients of the best model based on adjusted R^2:")
print(best_adjr2_model)

# Coefficients of the model with the lowest BIC
best_bic_model <- coef(best_subset, optimal_bic_size)
print("Coefficients of the best model based on BIC:")
print(best_bic_model)

# c)
print("Determination coefficient of the optimal adjusted R^2:")
print((summary(best_subset)$rsq[optimal_adjr2_size]))

print("Determination coefficient of the optimal BIC")
print((summary(best_subset)$rsq[optimal_bic_size]))

# d)
# Forward Selection
forward_selection <- regsubsets(Salary ~ ., data = data, nvmax = 16, method = "forward")
forward_summary <- summary(forward_selection)

# Optimal models from forward selection
optimal_forward_adjr2_size <- which.max(forward_summary$adjr2)
optimal_forward_bic_size <- which.min(forward_summary$bic)

best_forward_adjr2_model <- coef(forward_selection, optimal_forward_adjr2_size)
best_forward_bic_model <- coef(forward_selection, optimal_forward_bic_size)

cat("Optimal model size based on adjusted R^2 (Forward Selection):", optimal_forward_adjr2_size, "\n")
cat("Optimal model size based on BIC (Forward Selection):", optimal_forward_bic_size, "\n")
print("Coefficients of the best model based on adjusted R^2 (Forward Selection):")
print(best_forward_adjr2_model)
print("Coefficients of the best model based on BIC (Forward Selection):")
print(best_forward_bic_model)

# Comparison of models
print("Comparison of Best Subset and Forward Selection Models based on Adjusted R^2:")
cat("Best Subset Adjusted R^2 Model Size:", optimal_adjr2_size, "\n")
cat("Forward Selection Adjusted R^2 Model Size:", optimal_forward_adjr2_size, "\n")
print("Best Subset Adjusted R^2 Model Coefficients:")
print(best_adjr2_model)
print("Forward Selection Adjusted R^2 Model Coefficients:")
print(best_forward_adjr2_model)

print("Comparison of Best Subset and Forward Selection Models based on BIC:")
cat("Best Subset BIC Model Size:", optimal_bic_size, "\n")
cat("Forward Selection BIC Model Size:", optimal_forward_bic_size, "\n")
print("Best Subset BIC Model Coefficients:")
print(best_bic_model)
print("Forward Selection BIC Model Coefficients:")
print(best_forward_bic_model)



# PART II: Cross-validation

# a) 
set.seed(1)
sample <- sample(c(TRUE, FALSE), nrow(data), replace = TRUE, prob = c(0.6, 0.4))
train <- data[sample, ] # training set
valid <- data[!sample, ] # validation set

best_subset <- regsubsets(Salary ~ ., train, nvmax = 19)
X_valid <- model.matrix(Salary ~ ., data = valid)

# Initialization of a vector that will contain the errors
errors <- rep(0, 16)

# Loop over all model sizes from 1 to 16
for (i in 1:16) {
  # Extract coefficients for model with i predictors
  coef_x <- coef(best_subset, i)
  # Predict response variable (Salary)
  pred_x <- X_valid[, names(coef_x)] %*% coef_x
  # Compute mean squared prediction error
  errors[i] <- mean((valid$Salary - pred_x)^2)
}

# Define model sizes
model_sizes <- 1:16

# Plot Errors
plot(model_sizes, errors, type = "b", xlab = "Number of Variables", ylab = "Errors", 
     main = "Errors vs. Number of Variables")
points(which.min(errors), min(errors), col = "yellow", cex = 2, pch = 20)



# b) Repeat cross-validation for two different seed values and plot the results
# We only need to run the code snippet for 2(a) changing the set.seed(1) for
# 2 and 3.


# d)
set.seed(1)
n_splits <- 500
max_model_size <- 16
errors_matrix <- matrix(0, nrow = n_splits, ncol = max_model_size)

for (split in 1:n_splits) {
  # Split the data into training and validation sets
  sample <- sample(c(TRUE, FALSE), nrow(data), replace = TRUE, prob = c(0.6, 0.4))
  train <- data[sample, ]
  valid <- data[!sample, ]
  
  # Fit the best subset selection model
  best_subset <- regsubsets(Salary ~ ., train, nvmax = max_model_size)
  X_valid <- model.matrix(Salary ~ ., data = valid)
  
  for (i in 1:max_model_size) {
    # Extract coefficients for the model with i predictors
    coef_x <- coef(best_subset, id = i)
    
    # Predict response variable (Salary)
    pred_x <- X_valid[, names(coef_x)] %*% coef_x
    
    # Compute mean squared prediction error
    errors_matrix[split, i] <- mean((valid$Salary - pred_x)^2)
  }
}

# Compute average errors over all splits
average_errors <- colMeans(errors_matrix)

# Plot the averaged errors as a function of model size
model_sizes <- 1:max_model_size
plot(model_sizes, average_errors, type = "b", pch = 19, col = "deeppink3", 
     xlab = "Model Size", ylab = "Average Prediction Error (MSE)", 
     main = "Averaged Prediction Errors vs. Model Size")

# Identify the optimal model size
optimal_model_size <- which.min(average_errors)

# Add a legend
legend("topright", legend = paste("Optimal Model Size:", optimal_model_size))


#(e)
#Repeat the 2(a) changing the set.seed(1) for 2 and 3.