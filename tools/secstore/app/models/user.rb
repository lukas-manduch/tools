class User < ApplicationRecord
  has_secure_password
  validates :email, presence: true, length: {minimum: 5}
  validates :password_digest, presence: true
end
