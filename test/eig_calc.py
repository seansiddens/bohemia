import numpy as np

def compute_eigenvalues(matrix):
    """
    Compute eigenvalues of a given matrix.
    
    Args:
    matrix (list of lists or numpy.ndarray): Input matrix
    
    Returns:
    numpy.ndarray: Array of eigenvalues
    """
    # Convert input to numpy array if it's not already
    A = np.array(matrix, dtype=complex)
    
    # Compute eigenvalues
    eigenvalues = np.linalg.eigvals(A)
    
    return eigenvalues

# Example usage
if __name__ == "__main__":
    # Example matrix (can be any size, non-symmetric, with complex values)
    matrix = [
        [1+1j, 2, 3],
        [4, 5-2j, 6],
        [7, 8, 9+3j]
    ]
    
    eigenvalues = compute_eigenvalues(matrix)
    print("Eigenvalues:")
    for ev in eigenvalues:
        print(f"{ev:.4f}")
