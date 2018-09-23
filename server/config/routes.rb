Rails.application.routes.draw do
  resources :maps
  resources :game_accounts
  resources :characters
  resources :accounts
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html
end
