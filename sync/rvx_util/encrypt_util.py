from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from base64 import b64encode, b64decode
import cryptography

# private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
# public_key = private_key.public_key()

def encrypt_data(data:str, public_key):
  encrypted_data = public_key.encrypt(
    data.encode(),
    padding.OAEP(
      mgf=padding.MGF1(algorithm=hashes.SHA256()),
      algorithm=hashes.SHA256(),
      label=None
    )
  )
  return b64encode(encrypted_data)

def decrypt_data(encrypted_message_base64, private_key):
  encrypted_data = b64decode(encrypted_message_base64)
  decrypted_data = private_key.decrypt(
    encrypted_data,
    padding.OAEP(
      mgf=padding.MGF1(algorithm=hashes.SHA256()),
      algorithm=hashes.SHA256(),
      label=None
    )
  )
  return decrypted_data.decode()

def sign_data(data:str, private_key):
  padding_instance = padding.PSS(mgf=padding.MGF1(hashes.SHA256()), salt_length=padding.PSS.MAX_LENGTH)
  return b64encode(private_key.sign(data.encode(), padding_instance, hashes.SHA256()))

def verify_data(data:str, signature_base64, public_key):
  signature = b64decode(signature_base64)
  padding_instance = padding.PSS(mgf=padding.MGF1(hashes.SHA256()), salt_length=padding.PSS.MAX_LENGTH)
  try:
    public_key.verify(signature, data.encode(), padding_instance, hashes.SHA256())
    return True
  except cryptography.exceptions.InvalidSignature:
    return False

def public_key_test():
  plain_text = 'Hello, World!'
  private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
  public_key = private_key.public_key()

  encrypted_data = encrypt_data(plain_text, public_key)
  print(f"Encrypted data: {encrypted_data}")

  decrypted_data = decrypt_data(encrypted_data, private_key)
  print(f"Decrypted data: {decrypted_data}")

def private_key_test():
  plain_text = 'Hello, World!'
  private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
  public_key = private_key.public_key()

  sign = sign_data(plain_text, private_key)
  result = verify_data(plain_text, sign, public_key)
  print(sign)
  print(result)

if __name__ =='__main__':
  public_key_test()
  private_key_test()
