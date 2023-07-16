Rails.application.routes.draw do
  # Define your application routes per the DSL in https://guides.rubyonrails.org/routing.html

  # Defines the root path route ("/")
  # root "articles#index"
  get "/", to: "homepage#index"
  get "/login", to: "homepage#log_in_get", as: "login"
  post "/login", to: "homepage#log_in_post"
end
